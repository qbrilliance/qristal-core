==========
Python API
==========

..
   This is the top-level layout of Python module documentation.
   We can use directives, e.g., 'autoclass', to specify where the 
   API documentation of a class/function should be located.
   We can also embed additional text before/after the API documentation.

The session class
-----------------

The `session` class provides a way to set up the experiment table. 

You can use the experiment table to run a list of quantum circuits under different conditions, such as number of measurement shots, backends, parameters, etc.


.. autoclass:: core::session
   :members:

The Circuit class
-----------------

The `Circuit` class represents a quantum circuit, i.e., an ordered sequence of [quantum gates and measurements](https://qristal.readthedocs.io/en/latest/rst/quantum_computing.html).

In addition to elementary gates, it also supports pre-built circuit templates for commonly-used algorithms, such as Quantum Fourier Transform (QFT), algebraic circuits, etc.

.. autoclass:: core::Circuit
   :members:

Noise Modelling
---------------

QB Qristal allows an end-user to implement noise models in Python. 

.. autoclass:: core::NoiseModel
   :members:

This `NoiseModel` can be constructed from the quantum device `NoiseProperties`.

.. autoclass:: core::NoiseProperties
   :members:

Additionally, users can use these builtin classes to construct commonly-used noise channels when building the `NoiseModel`.

.. autoclass:: core::AmplitudeDampingChannel
   :members:

.. autoclass:: core::PhaseDampingChannel
   :members:

.. autoclass:: core::DepolarizingChannel
   :members:

.. autoclass:: core::GeneralizedPhaseAmplitudeDampingChannel
   :members:

.. autoclass:: core::GeneralizedAmplitudeDampingChannel
   :members:

.. autoclass:: core::ReadoutError
   :members:

In case no builtin noise channels are available for your use case, a fully-customized noise channel can be constructed in terms of instances of the `KrausOperator` class.

.. autoclass:: core::KrausOperator
   :members:

Placement
---------

Qristal contains placement methods to perform mapping from program (logical) qubits to device (physical) qubits satisfying qubit connectivity constraints.


Noise-aware placement
^^^^^^^^^^^^^^^^^^^^^

The `noise_aware_placement_pass` takes into account gate error rates and readout errors to find the best placement map.

.. autoclass:: core::noise_aware_placement_pass
   :members:

Device configuration for the noise-aware placement is defined by `noise_aware_placement_config`.

.. autoclass:: core::noise_aware_placement_config
   :members:

Swap-based placement
^^^^^^^^^^^^^^^^^^^^

The `swap_placement_pass` performs circuit placement by swapping qubits (along the shortest possible path) when there are gates between uncoupled qubits.

.. autoclass:: core::swap_placement_pass
   :members:
