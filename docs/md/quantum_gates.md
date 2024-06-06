This section provides a quick reference on common quantum gates.

- I - Identity
    - Unitary Description and Decomposition Rules
        
        This operator leaves a qubit unchanged, so that
        
         $|0\rangle \rightarrow |0\rangle$ and $|1\rangle \rightarrow |1\rangle$. 
        
        It is expressed by the matrix  $I = \left(\begin{array}{cc} 1 & 0 \\ 0 & 1\end{array} \right)$, but implemented by the absence of any operator.
        
- H - Hadamard
    - Unitary Description and Decomposition Rules
        
        This operator mixes the measurement basis states, so that
        
         $|0\rangle \rightarrow \frac{1}{\sqrt{2}}\Big(|0\rangle + |1\rangle\Big)$ and $|1\rangle \rightarrow \frac{1}{\sqrt{2}}\Big(|0\rangle - |1\rangle\Big)$. 
        
        It is given by the matrix  $H = \frac{1}{\sqrt{2}} \left(\begin{array}{rr} 1 & 1 \\ 1 & -1 \end{array} \right)$.
        
    - OpenQASM
        
        Gate format: `h <target_qubit>;`
        
    - XASM
        
        Gate format: `H(<target_qubit>);`
        
    - QB Native Transpilation
        
        ```python
        h(q0)
        ```
        
        where q0 is the qubit.
        
- X - Pauli rotation around the x-axis by pi
    - Unitary Description and Decomposition Rules
        
        This operator performs an X, or NOT operation on a qubit, turning
        
         $|0\rangle \rightarrow |1\rangle$ and $|1\rangle \rightarrow |0\rangle$. 
        
        It is given by the matrix  $X = \left(\begin{array}{cc} 0 & 1 \\ 1 & 0\end{array} \right)$.
        
    - OpenQASM
        
        Gate format: `x <target_qubit>;`
        
    - XASM
        
        Gate format: X`(<target_qubit>);`
        
    - QB Native Transpilation
        
        ```python
        x(q0)
        ```
        
        where q0 is the qubit.
        
- Y - Pauli rotation around the y-axis by pi
    - Unitary Description and Decomposition Rules
        
        This operator performs a Y operation on a qubit, turning
        
         $|0\rangle \rightarrow -i|1\rangle$ and $|1\rangle \rightarrow i\|0\rangle$. 
        
        It is given by the matrix  $Y = \left(\begin{array}{cc} 0 & -i \\ i & 0\end{array} \right)$
        
    - OpenQASM
        
        Gate format: `y <target_qubit>;`
        
    - XASM
        
        Gate format: Y`(<target_qubit>);`
        
    - QB Native Transpilation
        
        ```python
        y(q0)
        ```
        
        where q0 is the qubit.
        
- Z - Pauli rotation around the z-axis by pi
    - Unitary Description and Decomposition Rules
        
        This operator performs a Z operation on a qubit, turning
        
         $|0\rangle \rightarrow |0\rangle$ and $|1\rangle \rightarrow |1\rangle$. 
        
        It is given by the matrix  $Z = \left(\begin{array}{cc} 1 & 0 \\ 0 & -1 \end{array} \right)$
        
    - OpenQASM
        
        Gate format: `z <target_qubit>;`
        
    - XASM
        
        Gate format: `Z(<target_qubit>);`
        
    - QB Native Transpilation
        
        ```python
        z(q0)
        ```
        
        where q0 is the qubit.
        
- CNOT - controlled-X
    - Unitary Description and Decomposition Rules
        
        For a given control qubit $q_0$ and target qubit $q_1$ this operator performs a X operation on the $q_1$ if $q_0$ equals one, but leaves $q_1$ unchanged, *i.e.* the identity operation, if $q_0$ equals zero. It is given by the matrix
        
        $CX_{2q_0 + q_1 + 1,2q_0 + q_1 + 1} = \text{diag}(I,X) = \left(\begin{array}{cccc}1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ 0 & 0 & 0 & 1 \\ 0 & 0 & 1 & 0\end{array} \right)$
        
    - OpenQASM
        
        Gate format: `cx <control_qubit> , <target_qubit>;`
        
        where q[0] is the control qubit and q[1] is the target qubit.
        
    - XASM
        
        Gate format: `CX(<control_qubit>,<target_qubit>);`
        
    - QB Native Transpilation
        
        ```python
        cx(q0, q1)
        ```
        
        where q0 is the control qubit and q1 is the target qubit.
        
- CY - controlled-Y
    - Unitary Description and Decomposition Rules
        
        For a given control qubit $q_0$ and target qubit $q_1$ this operator performs a Y operation on the $q_1$ if $q_0$ equals one, but leaves $q_1$ unchanged, *i.e.* the identity operation, if $q_0$ equals zero. It is given by the matrix
        
        $CY_{2q_0 + q_1 + 1,2q_0 + q_1 + 1} = \text{diag}(I,Y) = \left(\begin{array}{cccc}1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ 0 & 0 & 0 & -i \\ 0 & 0 & i & 0\end{array} \right)$
        
        where $i^2 = -1$.
        
    - OpenQASM
        
        Gate format: `cy <control_qubit> , <target_qubit>;`
        
    - XASM
        
        Gate format: `CY(<control_qubit>,<target_qubit>);`
        
    - QB Native Transpilation
        
        ```python
        cy(q0, q1)
        ```
        
        where q0 is the control qubit and q1 is the target qubit.
        
- CZ - controlled-Z
    - Unitary Description and Decomposition Rules
        
        For a given control qubit $q_0$ and target qubit $q_1$ this operator performs a Z operation on the $q_1$ if $q_0$ equals one, but leaves $q_1$ unchanged, *i.e.* the identity operation, if $q_0$ equals zero. It is given by the matrix
        
        $CZ_{2q_0 + q_1 + 1,2q_0 + q_1 + 1} = \text{diag}(I,Z) = \left(\begin{array}{cccc}1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ 0 & 0 & 1 & 0 \\ 0 & 0 & 0 & -1\end{array} \right)$
        
    - OpenQASM
        
        Gate format: `cz <control_qubit> , <target_qubit>;`
        
    - XASM
        
        Gate format: `CZ(<control_qubit>,<target_qubit>);`
        
    - QB Native Transpilation
        
        ```python
        cz(q0, q1)
        ```
        
        where q0 is the control qubit and q1 is the target qubit.
        
- $R_x(\theta)$ - Pauli rotation on *X*-axis by $\theta$
    - Unitary Description and Decomposition Rules
        
        This operator rotates a qubit around the *X*-axis of the Bloch sphere by an angle of $\theta$. It partially interchanges $|0\rangle$ with $|1\rangle$ and introduces a relative phase between the two components, so
        
         $|0\rangle \rightarrow \cos\left(\frac{\theta}{2}\right)|0\rangle -i \sin\left(\frac{\theta}{2}\right) |1\rangle$   and    $|1\rangle \rightarrow -i\sin(\frac{\theta}{2})|0\rangle + \cos(\frac{\theta}{2})|1\rangle$. 
        
        It is given by the matrix 
        
         $X(\theta) = I \cos\!\left(\frac{\theta}{2}\right) -i \sigma_X  \sin\!\left(\frac{\theta}{2}\right) = \left(\begin{array}{cc} \cos\left(\frac{\theta}{2}\right) & -i\sin\left(\frac{\theta}{2}\right) \\ & \\-i\sin\left(\frac{\theta}{2}\right) & \cos\left(\frac{\theta}{2}\right)\end{array} \right)$,
        
        where $I$ is the identity matrix and $\sigma_X$ is the Pauli matrix
        
        $\sigma_X = \left(\begin{array}{cc} 0 & 1 \\ 1 & 0 \end{array}\right)$.
        
    - OpenQASM
        
        Gate format: `rx(<theta>) <target_qubit>;`
        
        where  `<theta>` is the angle of rotation about the X-axis.
        
    - XASM
        
        Gate format: `RX(<target_qubit>, <theta>);`
        
    - QB Native Transpilation
        
        ```python
        rx(q0,_theta)
        ```
        
        where q0 is the qubit and _*theta* is the angle of rotation about the X-axis.
        
- $R_y(\phi)$ - Pauli rotation on *Y*-axis by $\phi$
    - Unitary Description and Decomposition Rules
        
        This operator rotates a qubit around the *Y*-axis of the Bloch sphere by an angle of $\phi$, partially interchanging $|0\rangle$ with $|1\rangle$, so
        
         $|0\rangle \rightarrow \cos\left(\frac{\phi}{2}\right)|0\rangle - \sin\left(\frac{\phi}{2}\right) |1\rangle$   and    $|1\rangle \rightarrow \sin(\frac{\phi}{2})|0\rangle + \cos(\frac{\phi}{2})|1\rangle$. 
        
        It is given by the matrix 
        
         $Y(\phi) = I \cos\!\left(\frac{\phi}{2}\right) -i \sigma_Y  \sin\!\left(\frac{\phi}{2}\right) = \left(\begin{array}{cc} \cos\left(\frac{\phi}{2}\right) & -\sin\left(\frac{\phi}{2}\right) \\ & \\ \sin\left(\frac{\phi}{2}\right) & \cos\left(\frac{\phi}{2}\right)\end{array} \right)$,
        
        where $I$ is the identity matrix and $\sigma_Y$ is the Pauli matrix
        
        $\sigma_Y = \left(\begin{array}{cc} 0 & -i \\ i & 0 \end{array}\right)$.
        
    - OpenQASM
        
        Gate format: `ry(<phi>) <target_qubit>;`
        
        where  `<phi>` is the angle of rotation about the Y-axis.
        
    - XASM
        
        Gate format: `RY(<target_qubit>, <phi>);`
        
    - QB Native Transpilation
        
        ```python
        ry(q0,_phi)
        ```
        
        where q0 is the qubit and *_phi* is the angle of rotation about the Z-axis.
        
- $R_z(\lambda)$ - Pauli rotation on *Z*-axis by $\lambda$
    - Unitary Description and Decomposition Rules
        
        This operator rotates a qubit around the *Z*-axis of the Bloch sphere by an angle of $\lambda$. It introduces a relative phase between the two components, so
        
         $|0\rangle \rightarrow -i\sin(\frac{\lambda}{2})|0\rangle$   and    $|1\rangle \rightarrow i\sin(\frac{\lambda}{2})|1\rangle$. 
        
        It is given by the matrix 
        
         $Z(\lambda) = I \cos\!\left(\frac{\lambda}{2}\right) -i \sigma_Z  \sin\!\left(\frac{\lambda}{2}\right) = \left(\begin{array}{cc} \cos\left(\frac{\lambda}{2}\right) -i\sin\left(\frac{\lambda}{2}\right) & 0 \\ & \\0 & \cos\left(\frac{\lambda}{2}\right) +  i\sin\left(\frac{\lambda}{2}\right) \end{array} \right)$,
        
        where $I$ is the identity matrix and $\sigma_Z$ is the Pauli matrix
        
        $\sigma_Z = \left(\begin{array}{cc} 1 & 0 \\ 0 & -1\end{array}\right)$.
        
    - OpenQASM
        
        Gate format: `rz(<lambda>) <target_qubit>;`
        
        where  `<lambda>` is the angle of rotation about the Z-axis.
        
    - XASM
        
        Gate format: `RZ(<target_qubit>, <lambda>);`
        
    - QB Native Transpilation
        
        ```python
        rz(q0,_lambda)
        ```
        
        where q0 is the qubit and *_lambda* is the angle of rotation about the Z-axis.
        
- S - rotation on z-axis by 0.5*pi
    - Unitary Description and Decomposition Rules
        
        This operator rotates a qubit around the *Z*-axis of the Bloch sphere by an angle of $\frac{\pi}{2}$. It introduces a relative phase between the two components, where
        
         $|0\rangle \rightarrow -i\sin(\frac{\pi}{4})|0\rangle$   and    $|1\rangle \rightarrow i\sin(\frac{\pi}{4})|1\rangle$. 
        
        It is given by the matrix 
        
         $S = Z\left(\frac{\pi}{2}\right) = I \cos\!\left(\frac{\pi}{4}\right) -i \sigma_Z  \sin\!\left(\frac{\pi}{4}\right) = \left(\begin{array}{cc} \cos\left(\frac{\pi}{4}\right) -i\sin\left(\frac{\pi}{4}\right) & 0 \\ & \\0 & \cos\left(\frac{\pi}{4}\right) +  i\sin\left(\frac{\pi}{4}\right) \end{array} \right)$,
        
        where $I$ is the identity matrix and $\sigma_Z$ is the Pauli matrix
        
        $\sigma_Z = \left(\begin{array}{cc} 1 & 0 \\ 0 & -1\end{array}\right)$.
        
    - OpenQASM
        
        ```python
        import math
        rz(0.5*math.pi) q[0];
        ```
        
        where q[0] is the qubit.
        
    - XASM
        
        ```python
        import math
        Rz(q[0],0.5*math.pi) 
        ```
        
        where q[0] is the qubit.
        
    - QB Native Transpilation
        
        ```python
        import math
        rz(q0,0.5*math.pi)
        ```
        
        where q0 is the qubit.
        
- T - rotation on z-axis by 0.25*pi
    - Unitary Description and Decomposition Rules
        
        This operator rotates a qubit around the *Z*-axis of the Bloch sphere by an angle of $\frac{\pi}{4}$. It introduces a relative phase between the two components, where
        
         $|0\rangle \rightarrow -i\sin(\frac{\pi}{8})|0\rangle$   and    $|1\rangle \rightarrow i\sin(\frac{\pi}{8})|1\rangle$. 
        
        It is given by the matrix 
        
         $S = Z\left(\frac{\pi}{4}\right) = I \cos\!\left(\frac{\pi}{8}\right) -i \sigma_Z  \sin\!\left(\frac{\pi}{8}\right) = \left(\begin{array}{cc} \cos\left(\frac{\pi}{8}\right) -i\sin\left(\frac{\pi}{8}\right) & 0 \\ & \\0 & \cos\left(\frac{\pi}{8}\right) +  i\sin\left(\frac{\pi}{8}\right) \end{array} \right)$,
        
        where $I$ is the identity matrix and $\sigma_Z$ is the Pauli matrix
        
        $\sigma_Z = \left(\begin{array}{cc} 1 & 0 \\ 0 & -1\end{array}\right)$.
        
    - OpenQASM
        
        ```python
        import math
        rz(0.25*math.pi) q[0];
        ```
        
        where q[0] is the qubit.
        
    - XASM
        
        ```python
        import math
        RZ(q[0],0.25*math.pi)
        ```
        
        where q[0] is the qubit.
        
    - QB Native Transpilation
        
        ```python
        import math
        rz(q0,0.25*math.pi)
        ```
        
        where q0 is the qubit.
        
- $U(\theta,\phi,\lambda)$ - arbitrary rotation
    - Unitary Description and Decomposition Rules
        
        If used for an **ansatz** that will require **automatic differentiation** (AD), replace $U$ with the decomposition:
        
        $U(\theta, \phi, \lambda) = R_y(-\frac{\pi}{2})*R_x(\phi)*R_y(\theta)*R_x(\lambda)*R_y(\frac{\pi}{2})$
        
        **Other useful expressions:**
        
        $R_x(\alpha) = U(\alpha, -\frac{\pi}{2}, \frac{\pi}{2})$

        $R_y(\alpha) = U(\alpha, 0, 0)$
                
        $R_z(\alpha) = U(0, \alpha, 0) = U(0, 0, \alpha)$
        
    - OpenQASM
        
        Gate format: `u(<theta>, <phi>, <lambda>) <target_qubit>;`
        
        ```cpp
        // Format: u(theta, phi, lambda) target_qubit
        
        __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[1];
        
        // theta = 0.2
        // phi = -0.25
        // lambda = 1.1
        u(0.2, -0.25, 1.1) q[0];
        
        measure q[0] -> c[0];
        }
        ```
        
    - XASM
        
        Gate format: `U(<target_qubit>, <theta>, <phi>, <lambda>);`
        
    - QB Native Transpilation
        
        We use the decomposition:
        
        $U(\theta, \phi, \lambda) := R_y(-\frac{\pi}{2})*R_x(\phi)*R_y(\theta)*R_x(\lambda)*R_y(\frac{\pi}{2})$
                
- Measurement
    - Description
        
        Measurement is inherently non-unitary. The physical operation selects one of the components of the given quantum registry at random, where the probability of any particular component being selected is given by the square of its amplitude. For this reason it is advisable to call multiple shots (Qristal's default is 1024) so that the distribution of possible outcomes is apparent.
        
    - OpenQASM
        
        ```python
        measure q[0] -> c[0]
        ```
        
        where q[0] is the qubit and c[0] is the corresponding classical bit to which it is measured.
        
    - XASM
        
        Gate format: `Measure(<target_qubit>);`
        
    - QB Native Transpilation
        
        ```python
        measure(q0, c0)
        ```
        
        where q0 is the qubit and c0 is the corresponding classical bit to which it is measured.
