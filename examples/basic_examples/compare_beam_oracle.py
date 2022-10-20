import qbos as qb
import numpy as np
import ast
tqb = qb.core()
tqb.acc = "qpp"
tqb.sn = 1024
tqb.qb12()
tqb.nooptimise = True
tqb.noplacement = True


##########################################################################
#Test 1: SA = |0111>, FA = |01>, FB = |01>

#Inputs
q0 = 0
q1 = 1
q2 = 2
FA = [3,4]
FB = [5,6]
SA = [7,8,9,10]

circ = qb.Circuit()

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
# result = circ.execute()
# res = ast.literal_eval(result)
# print(res)
# assert("010" in res["AcceleratorBuffer"]["Measurements"])
tqb.ir_target = circ
tqb.run()
print(tqb.out_raw[0])
result = tqb.out_raw[0][0]
res = ast.literal_eval(result)
assert(res["010"] == 1024) #assert("010" in list(res.keys()))

##########################################################################
#Test 2: SA = |1111>, FA = |01>, FB = |01>

#Inputs
q0 = 0
q1 = 1
q2 = 2
FA = [3,4]
FB = [5,6]
SA = [7,8,9,10]

circ = qb.Circuit()

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
# result = circ.execute()
# res = ast.literal_eval(result)
# print(res)
# assert("111" in res["AcceleratorBuffer"]["Measurements"])
tqb.ir_target = circ
tqb.run()
print(tqb.out_raw[0])
result = tqb.out_raw[0][0]
res = ast.literal_eval(result)
assert(res["111"] == 1024) #assert("010" in list(res.keys()))