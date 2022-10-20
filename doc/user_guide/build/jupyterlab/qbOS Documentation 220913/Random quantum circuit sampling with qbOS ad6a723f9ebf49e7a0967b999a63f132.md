# Random quantum circuit sampling with qbOS

qbOS can construct (pseudo-)random circuits of single- and two-qubit gates and, therefore, get utilised for *random quantum circuit sampling* experiments (more contents coming soon). In the simple example below, we generate a random circuit of the minimum depth of 2, which demonstrate how a relatively complicated task can be performed in just a few lines of high-level codes.  

```python
import qbos as q

tqb = q.core()
tqb.qb12()

tqb.random = 2

tqb.run()
```

**Counts** can be viewed using:

```python
tqb.out_raw[0]
```

The result is (similar to) this:

```python
String[{
    "000000000000": 511,
    "001000000000": 513
}]
```

An alternative way of seeing the counts (expressing the bit string key as an integer, and the count values also as integers):

```python
tqb.out_count[0]
```

The result is (similar to) this:

```python
[{0: 511, 512: 513}]
```

To **view the random circuit** that was generated in an OpenQASM format:

```python
tqb.instring[0]
```

The result will be (similar to) this:

```python
String[
# Random circuit created:

__qpu__ void QBCIRCUIT(qreg q) {
  OPENQASM 2.0;
  include "qelib1.inc";
  creg c[12];
  u1(1.74458) q[2];
  cu1(2.18845) q[5],q[7];
  cu3(2.9196,0.276159,2.24471) q[3],q[1];
  cx q[8],q[11];
  cz q[6],q[4];
  crz(2.37187) q[10],q[0];
  u3(1.54745,1.01206,2.92662) q[9];
  measure q[0] -> c[0];
  measure q[1] -> c[1];
  measure q[2] -> c[2];
  measure q[3] -> c[3];
  measure q[4] -> c[4];
  measure q[5] -> c[5];
  measure q[6] -> c[6];
  measure q[7] -> c[7];
  measure q[8] -> c[8];
  measure q[9] -> c[9];
  measure q[10] -> c[10];
  measure q[11] -> c[11];
}
]
```