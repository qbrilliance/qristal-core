# GUI circuit composition with pyQuirk

## Importing the module

```python
import qbos
import qbos_vis

tqb = qbos.core()
tqb.qb12()

qbos_vis.quirk
```

![Untitled](GUI%20circuit%20composition%20with%20pyQuirk%20f7161f17d431491c9497e928d3486cdc/Untitled.png)

## Drag-and-drop to construct a circuit

![GUI%20circuit%20composition%20with%20pyQuirk%20f7161f17d431491c9497e928d3486cdc/Untitled%201.png](GUI%20circuit%20composition%20with%20pyQuirk%20f7161f17d431491c9497e928d3486cdc/Untitled%201.png)

## Running the circuit in qbOS

```python
tqb.instring = qbos_vis.quirk.circuit_qasm
tqb.run()
```

Check the results:

```python
tqb.out_raw[0]
```

## Troubleshooting

- When pyQuirk fails to start
    
    If you were running JupyterLab 2 (i.e. a release prior to 210627), you should first **terminate the already running JupyterLab container**.  To do this, use the command below in your SSH login terminal:
    
    `sudo docker stop qbemp`
    
    Then log out of your SSH terminal, and then re-login with SSH.  This should trigger a new container that has JupyterLab 3 and pyQuirk automatically set up.  
    
    Note that it takes around 2 minutes for the new container to start.
    
- pyQuirk crashes
    
    Check your Measurement gates - avoid having more than one Measurement in a column.