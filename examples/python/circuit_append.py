import qristal.core

c1 = qristal.core.Circuit() 
c1.h(0)
print(c1.openqasm())

c2 = qristal.core.Circuit() 
c2.rx(0, 1.23)
print(c2.openqasm())

c1.append(c2)
print(c1.openqasm())