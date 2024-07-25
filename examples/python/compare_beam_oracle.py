import qb.core
import numpy as np
s = qb.core.session()
s.acc = "qpp"
s.sn = 1024
s.init()
s.nooptimise = True
s.noplacement = True


##########################################################################
#Test 1: SA = |0111>, FA = |01>, FB = |01>

#Inputs
q0 = 0
q1 = 1
q2 = 2
FA = [3,4]
FB = [5,6]
SA = [7,8,9,10]

circ = qb.core.Circuit()

circ.x(FA[1])
circ.x(FB[1])
for i in range(1,len(SA)):
  circ.x(SA[i])

#Add beam comparator
circ.compare_beam_oracle(q0, q1, q2, FA, FB, SA)

#Measure flags
circ.measure(q0)
circ.measure(q1)
circ.measure(q2)
# circ.print()

#Run
s.ir_target = circ
s.run()
print(s.results[0][0])
assert(s.results[0][0][(0,1,0)] == 1024)

##########################################################################
#Test 2: SA = |1111>, FA = |01>, FB = |01>

#Inputs
q0 = 0
q1 = 1
q2 = 2
FA = [3,4]
FB = [5,6]
SA = [7,8,9,10]

circ = qb.core.Circuit()

circ.x(FA[1])
circ.x(FB[1])
for i in range(0,len(SA)):
  circ.x(SA[i])

#Add beam comparator
circ.compare_beam_oracle(q0, q1, q2, FA, FB, SA)

#Measure flags
circ.measure(q0)
circ.measure(q1)
circ.measure(q2)

#Run
s.ir_target = circ
s.run()
print(s.results[0][0])
assert(s.results[0][0][[1,1,1]] == 1024)
