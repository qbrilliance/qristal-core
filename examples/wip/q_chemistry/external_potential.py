import openfermionpyscf as ofpyscf

import numpy as np
from ase.units import Bohr

class PointChargePotential():
    def __init__(self, charges, positions=None):
        """ Parameters
        charges: list of float
            Charges.
        positions: (N, 3)-shaped array-like of float
            Positions of charges in Angstrom.  Can be set later.

        Example implementation of this class
        https://gitlab.com/gpaw/gpaw/-/blob/master/gpaw/external.py
        """
        self._dict = dict(name=self.__class__.__name__,
                          charges=charges, positions=positions)
        self.q_p = np.ascontiguousarray(charges, float)
        if positions is not None:
            self.set_positions(positions)
        else:
            self.R_pv = None


    def set_positions(self, R_pv, com_pv=None):
        """Update positions."""
        if com_pv is not None:
            self.com_pv = np.asarray(com_pv) / Bohr
        else:
            self.com_pv = None

        self.R_pv = np.asarray(R_pv) / Bohr

    def get_perturb_ints(self, mol):
        """ get one electron integrals sum_I < | -Q_I/|r-r_I| | >
        in atomic orbital basis
        Note: must convert to MO basis 
        """
        one_body_integrals = 0
        for point_id in range(len(self.q_p)):
            with mol.with_rinv_origin(self.R_pv[point_id]):
                rinv = mol.intor("int1e_rinv", comp=1)
                rinv *= -self.q_p[point_id]
                one_body_integrals += rinv
        return one_body_integrals

    def get_drinv_integrals(self, mol, point_id):
        """ get one electron integrals d/dr_I < | -Q_I/|r-r_I| | >
        Note: must convert to MO basis 
        """
        with mol.with_rinv_origin(self.R_pv[point_id]):
            drinv = mol.intor("int1e_drinv", comp=3)
            drinv *= -self.q_p[point_id]
        return drinv

    def get_MM_operator(self, calc):
        """ get correction to QM hamiltonian due to the point charges 
        returns a OpenFermion InteractionOperator
        """
        mol = calc.molecule._pyscf_data['mol']
        one_body_integrals = self.get_perturb_ints(mol)
        one_body_integrals = calc.ao_to_mo(one_body_integrals)
        M = one_body_integrals.shape[1]
        two_body_integrals = np.zeros((M,M,M,M))
        nuc_repulsion = 0

        MM_operator = calc.get_molecular_hamiltonian(calc.molecule, nuc_repulsion, 
            one_body_integrals, two_body_integrals)
        return MM_operator

    def get_ngrad_nn(self, mol, calc, atmlst=None):
        """ Derivatives wrt nuclear coordinates of coulomb potential 
        between QM nuclei and point charges 
        Similar to pyscf.grad.rhf.grad_nuc and self.get_pgrad_nn
        """
        gs = np.zeros((mol.natm,3))
        for j in range(mol.natm):
            q2 = mol.atom_charge(j)
            r2 = mol.atom_coord(j)
            for i in range(len(self.q_p)):
                q1 = self.q_p[i]
                r1 = self.R_pv[i]
                r = np.sqrt(np.dot(r1-r2,r1-r2))
                gs[j] -= q1 * q2 * (r2-r1) / r**3
        if atmlst is not None:
            gs = gs[atmlst]
        return gs
    
    def get_pgrad_nn(self, mol, calc, atmlst=None):
        """ Derivatives wrt point charge coordinates of coulomb potential 
        between QM nuclei and point charges 
        Similar to pyscf.grad.rhf.grad_nuc
        """
        gs = np.zeros((len(self.q_p),3))
        for j in range(len(self.q_p)):
            q2 = self.q_p[j]
            r2 = self.R_pv[j]
            for i in range(mol.natm):
                q1 = mol.atom_charge(i)
                r1 = mol.atom_coord(i)
                r = np.sqrt(np.dot(r1-r2,r1-r2))
                gs[j] -= q1 * q2 * (r2-r1) / r**3
        if atmlst is not None:
            gs = gs[atmlst]
        return gs

    def get_forces(self, calc):
        """Calculate forces from QM charge density on point-charges."""
        mol = calc.molecule._pyscf_data['mol']
        grad_nn = self.get_pgrad_nn(mol, calc)
        M = grad_nn.shape[0]
        two_body_integrals = np.zeros((M,M,M,M))
        forces = np.zeros((len(self.q_p), 3))
        for point_id in range(len(self.q_p)):
            one_body_integrals = self.get_drinv_integrals(mol, point_id)
            one_body_integrals = [calc.ao_to_mo(one_body_integrals[i]) for i in range(3)]
            for i in range(3):
                # create and evaluate operator for each coordinate
                grad_ham = calc.get_molecular_hamiltonian(calc.molecule, grad_nn[point_id][i], 
                    one_body_integrals[i], two_body_integrals)
                grad_ham_jw = calc.squant_to_pauli(grad_ham)
                # measure pauli terms in operator using VQE circuit to obtain force
                forces[point_id][i] = -calc.evaluate_VQE(grad_ham_jw)

        return forces
