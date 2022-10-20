# GUI circuit composition and conversion to QPU kernel for execution in qbOS

# 1.0 Background

Many users who have studied quantum computing will have used drag-and-drop GUI tools to perform quantum circuit composition.  This article is a quick guide on how to **export a finished circuit**, followed by how to make the small **modifications needed to turn the exported circuit into a QPU kernel** format that qbOS can then run.

# 2.0 Export from IBM Quantum Experience

 Here is a screenshot from the [IBM Quantum Experience](https://quantum-computing.ibm.com/) circuit builder:

![GUI%20circuit%20composition%20and%20conversion%20to%20QPU%20kern%204f46ef713a5743e192aa79ec6f804d5d/Screen_Shot_2020-09-28_at_3.09.33_pm.png](GUI%20circuit%20composition%20and%20conversion%20to%20QPU%20kern%204f46ef713a5743e192aa79ec6f804d5d/Screen_Shot_2020-09-28_at_3.09.33_pm.png)

To export your finished composition, click in the Code editor (QASM format) → ... → **Copy code**

# 3.0 Paste circuit into JupyterLab

In an open Jupyter notebook:

```python
import re
import qbos

my_importc = '''
remove this line and paste your copied circuit (ctrl-v)

'''
```

# 4.0 Modifications to make a QPU kernel

Remove `qreg` declaration from the body of the OpenQASM code:

```python
q2oqm = re.sub(r"qreg q\[\d+\];", "", my_importc)
```

Next, add a QPU kernel declaration to enclose the OpenQASM code:

```python
qbstr = '__qpu__ void QBCIRCUIT(qreg q) {\n' + q2oqm + '}'
```

# 5.0 Execute with qbOS

```python
tqb = qbos.core()
tqb.qb12()
tqb.instring = qbstr
tqb.run()
```

# 6.0 Inspect the shot outcomes

```python
tqb.out_raw[0]

```