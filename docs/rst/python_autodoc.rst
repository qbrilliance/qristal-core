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

The `Circuit` class represents a quantum circuit, i.e., an ordered sequence of quantum gates and measurements.

In addition to elementary gates, it also supports pre-built circuit templates for commonly-used algorithms, such as Quantum Fourier Transform (QFT), algebraic circuits, etc.

.. autoclass:: core::Circuit
   :members: