import os, xacc, numpy as np
# Number of Hydrogens
N = 4

# Construct the geometry string
x = 0.0
geom = ''
for i in range(N):
    geom += ("H 0.0 0.0 " + str(x))
    if (i != N - 1): 
      geom += "; "
    if (i % 2 == 0):
      x += 0.75
    else:
      x += 1.125
    
# Construct the Hamiltonian using PySCF
H_fermion = xacc.getObservable('pyscf', {'basis': 'sto-3g', 'geometry': geom})
H = xacc.transformToPauli('jw', H_fermion)
print(H.toString())

# Build up the binary data rep for NVQ++
n_terms = 0
data = []
for _, term in H:
   print(term.ops(), term.coeff())
   real_part = np.real(term.coeff())
   sub_data = [0]*2*N
   for i, o in term.ops().items():
       if o == 'X':
           sub_data[i] = 1
       elif o == 'Y':
           sub_data[i] = 3
       elif o == 'Z':
           sub_data[i] = 2
   for el in sub_data:
       data.append(el)
   data.append(real_part)
   data.append(0.0)
   n_terms = n_terms + 1
 
data.append(n_terms)
# print(len(data), data)
 
# Dump it to a binary file
from array import array
dir_path = os.path.dirname(os.path.realpath(__file__))
file_name = dir_path + '/h2_{}_terms_data.bin'.format(n_terms)
output_file = open(file_name, 'wb')
float_array = array('d', data)
float_array.tofile(output_file)
output_file.close()
print('Wrote binary file: ', file_name)
