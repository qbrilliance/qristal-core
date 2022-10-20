# Copyright (c) 2021 Quantum Brilliance Pty Ltd
import os
import pytest
def test_CI_211101_1_q_chemistry_h2() :
    print("* CI_211101_1_q_chemistry_h2:")
    print("* Runs VQE to determine the H-H optimal bond length (in Bohr)")
    from ase import Atoms
    from ase.optimize import BFGS
    from vqe_calculator import VQE
    from ase.units import Bohr

    # bond length in Bohr
    rAB = 2.1
    # position values in Angstrom
    atoms = Atoms('HH', positions=[[0, 0, 0], [0, 0, rAB*Bohr]])
    vqe_params = {"acc":"qpp", "theta":[.08]*6+[.08,1.5,2.1], "ansatz":"aswap", 
    "maxeval":500, "functol":1e-5, "method":"nelder-mead", "sn":0, "addqubits":1}
    calc = VQE(basis='sto3g', n_active_electrons=None, n_active_orbitals=None, vqe_params=vqe_params)
    atoms.calc = calc

    opt = BFGS(atoms, trajectory='opt.traj')
    # fmax is the maximum force on any atom when the optimizer finishes
    opt.run(fmax=0.002)
    optimum_bond_len = atoms.get_distance(0,1)/Bohr
    assert optimum_bond_len == pytest.approx(1.388, None, 0.001)

def test_another_test():
    print("* TBD ")
