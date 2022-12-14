from ase.calculators.calculator import Calculator
from external_potential import PointChargePotential
from typing import Any, Dict
import re
import subprocess
import numpy as np
from functools import reduce

import openfermion as of
import openfermionpyscf as ofpyscf
from pyscf import scf, grad, gto, data
import qb.core
import qb.core.optimization

BOHR =  data.nist.BOHR

def run_vqe(qn=4, acc="qpp", ham="0", theta=[.08,1.5,2.1], ansatz="aswap", aswapn=6,
        maxeval=201, functol=1e-5, method="nelder-mead", toprint=False, sn=0, addqubits=0):
    ham = change_index(ham, addqubits)
    qn += addqubits
    vqe = qb.op.vqe()
    vqe.theta = qb.core.ND()
    attrs = ['qn', 'acc','ham', 'ansatz', 'aswapn', 'method', 'maxeval']
    vals = [qn, acc, ham, ansatz, aswapn, method, maxeval]
    vqe.sn = sn
    for attr, val in zip(attrs, vals):
        setattr(vqe, attr, val)

    # TODO: fill in theta with required number of parameters
    for i in range(len(theta)):
        vqe.theta[0][0][i] = theta[i]

    if get_n_qubits(ham)==0:
        return (float(ham), 0)
    elif not vqe.out_energy[0][0]:
        vqe.run()
    if toprint: print(vqe)
    return (vqe.out_energy[0][0][0], [vqe.out_theta[0][0][x] for x in range(len(theta))])

def get_n_qubits(ham : str) -> int:
    ''' Find the number of qubits required by Hamiltonian string
    by finding the highest label number in a pauli operator
    '''
    indexes = re.findall(r'(?<=[XYZ])[0-9]+|/g', ham)
    q = max(list(map(int, indexes)), default=-1)
    return q+1

def change_index(q_ham : str, val=1) -> str:
    # Increase the indices of Pauli operators by a set amount
    return re.sub(r'(?<=[XYZ])[0-9]+|/g', lambda x: str(int(x.group())+val), q_ham)

class VQE(Calculator):
    """This is the ASE-calculator frontend for calculating molecular
    properties, implementing the Calculator interface

    Calculator interface source code
    https://gitlab.com/ase/ase/-/blob/master/ase/calculators/calculator.py
    """

    implemented_properties = ['energy', 'forces']

    default_parameters : Dict[str, Any] = {
        'basis' : 'sto3g',  # can also be a dict
        'multiplicity': None,  # spin multiplicity 2S+1
        'n_active_electrons': None,  # electrons in active space
        'n_active_orbitals': None,  # spatial orbitals in active space
        'verbose': False,
        'vqe_params': {}
    }

    def __init__(self, **kwargs):
        self.molecule = None
        self.occupied_indices = None
        self.active_indices = None
        self.optimized_theta = None
        self.pc = None
        self.to_calculate = True
        Calculator.__init__(self, **kwargs)

    def calculate(self, atoms=None, properties=['energy'],
                  system_changes=['cell']):
        """Do the calculation.

        properties: list of str
            List of what needs to be calculated.  Can be any combination
            of 'energy', 'forces', 'stress', 'dipole', 'charges', 'magmom'
            and 'magmoms'.
        system_changes: list of str
            List of what has changed since last calculation.  Can be
            any combination of these six: 'positions', 'numbers', 'cell',
            'pbc', 'initial_charges' and 'initial_magmoms'.

        Calculated properties are inserted into results dictionary like
        shown in this dummy example::

            self.results = {'energy': 0.0,
                            'forces': np.zeros((len(atoms), 3)),
                            'stress': np.zeros(6),
                            'dipole': np.zeros(3),
                            'charges': np.zeros(len(atoms)),
                            'magmom': 0.0,
                            'magmoms': np.zeros(len(atoms))}

        """
        Calculator.calculate(self, atoms, properties, system_changes)

        if system_changes or self.to_calculate:
            self.to_calculate = False
            # Set molecule parameters
            geometry = list(zip(atoms.get_chemical_symbols(), atoms.get_positions()))
            basis = self.parameters['basis']
            charge = int(sum(atoms.get_initial_charges()))
            multiplicity = self.parameters['multiplicity']
            if multiplicity is None: # maximally pair electrons
                multiplicity = int((sum(atoms.get_atomic_numbers()) - charge)) % 2 + 1
            n_active_electrons = self.parameters['n_active_electrons']
            n_active_orbitals = self.parameters['n_active_orbitals']

            # Perform electronic structure calculations and
            # obtain Hamiltonian as an OpenFermion InteractionOperator
            self.molecule = of.MolecularData(geometry, basis, multiplicity, charge)
            self.occupied_indices, self.active_indices = self.get_occupied_indices(self.molecule,
                    n_active_electrons, n_active_orbitals)
            one_body_integrals, two_body_integrals = self.get_integrals(self.molecule)
            hamiltonian = self.get_molecular_hamiltonian(self.molecule,
                    float(self.molecule._pyscf_data['mol'].energy_nuc()),
                    one_body_integrals, two_body_integrals)

            # perform JW transform on second quantized hamiltonian
            hamiltonian_jw_str = self.squant_to_pauli(hamiltonian)

            # perform VQE to determine ground state
            if n_active_electrons is not None:
                self.parameters['vqe_params']['aswapn'] = n_active_electrons
            else:
                self.parameters['vqe_params']['aswapn'] = self.molecule.n_electrons
            n_qubits = get_n_qubits(hamiltonian_jw_str)
            energy, self.optimized_theta = run_vqe(qn=n_qubits, ham=hamiltonian_jw_str,
                toprint=self.parameters['verbose'], **self.parameters['vqe_params'])
            self.results['energy'] = energy
            # print("Hartree-Fock energy:", self.molecule._pyscf_data['scf'].e_tot)
            # print("VQE energy:         ", energy)

        if 'forces' in properties:
            # calculate derivative of hamiltonian wrt each nuclear coordinate
            # return array of dimension (M, 3)
            mol = self.molecule._pyscf_data['mol']
            grad_nn = grad.rhf.grad_nuc(mol)  # derivatives of nuc-nuc repulsion shape=(M,3)
            if self.pc is not None:
                # get coulomb forces on nuclei due to external point charges
                grad_nn += self.pc.get_ngrad_nn(mol, self)

            forces = np.zeros((len(atoms), 3))
            # grad.rhf.grad_elec(mf_grad) + grad.rhf.grad_nuc(mol) to get HF forces
            for atm_id in range(len(atoms)):
                # derivative of one and two electron integrals wrt to coordinates of atm_id
                one_body_integrals, two_body_integrals = self.fd_grad_integrals(self.molecule, atm_id)
                for i in range(3):
                    # create operator for each component of nuclear coordinate
                    grad_ham = self.get_molecular_hamiltonian(self.molecule, grad_nn[atm_id][i],
                        one_body_integrals[i], two_body_integrals[i])
                    grad_ham_jw = self.squant_to_pauli(grad_ham)
                    # measure pauli terms in operator using VQE circuit to obtain force
                    forces[atm_id][i] = -self.evaluate_VQE(grad_ham_jw)

            self.results['forces'] = forces


    def fd_grad_integrals(self, molecule, atm_id : int):
        # gradient of AO integrals using central finite difference
        h = 0.00001*BOHR
        one_body_integrals = np.array([None, None, None])
        two_body_integrals = np.array([None, None, None])
        for i in range(3):
            perturb = np.array([0., 0., 0.])
            np.put(perturb, i, h)
            ints1 = self.get_integrals(molecule, atm_id, perturb)
            ints2 = self.get_integrals(molecule, atm_id, -perturb)
            one_body_integrals[i] = (ints1[0] - ints2[0])/(2*h)*BOHR
            two_body_integrals[i] = (ints1[1] - ints2[1])/(2*h)*BOHR
        return one_body_integrals, two_body_integrals

    def get_integrals(self, molecule, atm_id = 0, perturb = np.array([0,0,0])):
        """ compute one and two electron integrals for a given molecule
        with a perturbation of chosen nuclei position
        Args:
            molecule: An instance of the OpenFermion MolecularData class.
            atm_id: Index of atom in molecule which is to be perturbed
            perturb: A numpy array representing displacement of coordinates of atom
        Returns:
            One and two body integrals of the molecule in MO basis
        """
        # build pyscf molecule and run scf
        mol = self.prepare_pyscf_molecule(molecule)
        mol.atom[atm_id] = (mol.atom[atm_id][0], mol.atom[atm_id][1]+perturb)
        mol.build()
        pyscf_scf = ofpyscf._run_pyscf.compute_scf(mol)
        # calculate mo_coeff using canonical orthogonalisation as opposed to
        # scf calculation from pyscf_scf.run()
        ortho = scf.canonical_orth_(pyscf_scf.get_ovlp(), thr=1e-9)
        ortho = np.flip(ortho, axis=1)
        pyscf_scf.mo_coeff = ortho
        if not hasattr(molecule, '_pyscf_data') and np.linalg.norm(perturb) == 0:
            molecule._pyscf_data = pyscf_data = {}
            pyscf_data['mol'] = mol
            pyscf_data['scf'] = pyscf_scf

        one_body_ints, two_body_ints = ofpyscf._run_pyscf.compute_integrals(mol, pyscf_scf)
        if self.pc is not None:
            # add perturbation to hamiltonian due to external point charges
            perturb_ints = self.pc.get_perturb_ints(mol)
            perturb_ints = self.ao_to_mo(perturb_ints, pyscf_scf)
            one_body_ints += perturb_ints
        return (one_body_ints, two_body_ints)

    def prepare_pyscf_molecule(self, molecule):
        """ Args:
                molecule: An instance of the OpenFermion MolecularData class.
            Returns:
                pyscf_molecule: A pyscf molecule instance.
        """
        pyscf_molecule = gto.Mole()
        # allow pyscf_molecule.atom to be modified without affecting molecule.geometry
        pyscf_molecule.atom = list(molecule.geometry)
        pyscf_molecule.basis = molecule.basis
        pyscf_molecule.spin = molecule.multiplicity - 1
        pyscf_molecule.charge = molecule.charge
        pyscf_molecule.symmetry = False

        return pyscf_molecule

    def get_occupied_indices(self, molecule, n_active_electrons, n_active_orbitals):
        # Freeze core orbitals and truncate to active space
        # adapted from ofpyscf.get_molecular_hamiltonian
        if n_active_electrons is None:
            n_core_orbitals = 0
            occupied_indices = None
        else:
            n_core_orbitals = (molecule.n_electrons - n_active_electrons) // 2
            occupied_indices = list(range(n_core_orbitals))

        if n_active_orbitals is None:
            active_indices = None
        else:
            active_indices = list(range(n_core_orbitals,
                                        n_core_orbitals + n_active_orbitals))
        return (occupied_indices, active_indices)

    def squant_to_pauli(self, hamiltonian):
        '''
        Args:
            ham: a second quantized operator of type
                of.ops.representations.InteractionOperator

        Returns:
            hamiltonian_jw_str: a string which represents the operator
                after the Jordan-Wigner transform
        '''
        # Convert to a FermionOperator
        hamiltonian_ferm_op = of.get_fermion_operator(hamiltonian)
        # Map to QubitOperator using the JWT
        hamiltonian_jw = of.jordan_wigner(hamiltonian_ferm_op)
        # remove all line breaks and brackets for SDK
        hamiltonian_jw_str = re.sub(r"\+0j|\r|\n|\[|\]|\(|\)|/g", "", str(hamiltonian_jw))
        return hamiltonian_jw_str

    def get_molecular_hamiltonian(self, molecule, nuc_repulsion,
            one_body_integrals, two_body_integrals):
        '''
        Args:
            molecule: OpenFermion MolecularData object
            nuc_repulsion: float
            one_body_integrals: shape = (M,M)
            two_body_integrals: shape = (M,M,M,M)
        Returns:
            Second quantized hamiltonian as an OpenFermion InteractionOperator object
        '''
        ofmolecule = of.MolecularData(molecule.geometry, molecule.basis,
                molecule.multiplicity, molecule.charge)
        ofmolecule.nuclear_repulsion = nuc_repulsion
        ofmolecule.one_body_integrals = one_body_integrals
        ofmolecule.two_body_integrals = two_body_integrals
        return ofmolecule.get_molecular_hamiltonian(self.occupied_indices, self.active_indices)

    def evaluate_VQE(self, ham : str, theta=None):
        '''
        Evaluate expectation of qubit hamiltonian on the ansatz with given
        optimized angles
        '''
        if theta is None:
            theta = self.optimized_theta
        n_qubits = get_n_qubits(ham)
        kwargs = dict(self.parameters['vqe_params'])
        if 'theta' in kwargs: kwargs.pop('theta')
        if 'maxeval' in kwargs: kwargs.pop('maxeval')
        # evaluate hamiltonian at given set of angles using one evaluation
        ev, _ = run_vqe(qn=n_qubits, ham=ham, toprint=False, theta=theta,
            maxeval = 1, **kwargs)
        return ev

    def embed(self, q_p):
        """ Embed QM region in point-charges. Positions can be set with
        set_positions function in PointChargePotential
        Arguments:
            q_p: List of floats indicating charges
        """
        pc = PointChargePotential(q_p)
        self.pc = pc
        self.to_calculate = True
        return pc

    def ao_to_mo(self, ao, pyscf_scf=None):
        """ Convert atomic orbital integrals to molecular orbitals
        using the MO coefficients from scf calculation. Works for one electron
        integrals only. Similar to ofpyscf.run_pyscf.compute_integrals
        Args:
            ao: atomic orbital integrals as a numpy array of shape (M,M)
        """
        if pyscf_scf is None:
            pyscf_scf = self.molecule._pyscf_data['scf']
        n_orbitals = pyscf_scf.mo_coeff.shape[1]
        one_electron_compressed = reduce(np.dot, (pyscf_scf.mo_coeff.T,
                                                    ao,
                                                    pyscf_scf.mo_coeff))
        one_electron_integrals = one_electron_compressed.reshape(
            n_orbitals, n_orbitals).astype(float)

        return one_electron_integrals
