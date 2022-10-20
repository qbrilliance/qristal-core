# Usage/examples of qbos.MapNC type

# 1.0 Background

A data type, `qbos.NC` is provided in qbOS for storing **maps**.  The key-value pairs for this type use integer keys.  The associated pair value is a double-precision complex value.   These maps are useful as storage for **sparse** vectors of complex values.

# 2.0 Creating elements (of type `qbos.NC` that map integer → double precision complex)

```python
import qbos

map_r1_c1 = qbos.NC()  # A map
map_r1_c2 = qbos.NC()  # A second map

map_r2_c1 = qbos.NC()  # A third map
map_r2_c2 = qbos.NC()  # A fourth map
```

Populate the maps:

```python
map_r1_c1[0] = 1.23+0.5j  # map_r1_c1: 0 -> 1.23+0.5j
map_r1_c1[1] = -0.5+0.2j  # map_r1_c1: 1 -> -0.5+0.2j
map_r1_c1[2] = 0.2-0.05j # map_r1_c1: 2 -> 0.2-0.05j

map_r1_c2[0] = 1.1+1.1j  # map_r1_c2: 0 -> 1.1+1.1j
map_r1_c2[1] = -0.8-0.5j  # map_r1_c2: 1 -> -0.8-0.5j
map_r1_c2[2] = 0.1-0.2j # map_r1_c2: 2 -> 0.1-0.2j

map_r2_c1[0] = 1.31  # map_r1_c1: 0 -> 1.31

map_r2_c2[0] = 2.1j # map_r2_c2: 0 -> 2.1j
```

Remove an entry from a map:

```python
map_r1_c1.__delitem__(2)  # map_r1_c1: key 2 removed
map_r1_c2.__delitem__(2)  # map_r1_c2: key 2 removed
```

Modify an entry in a map:

```python
map_r1_c1[1] += 0.05  # map_r1_c1: key 1 increment value by 0.05
map_r1_c2[1] -= 0.1 # map_r1_c2: key 1 decrement value by 0.1
```

# 3.0 Assemble elements into a list, `qbos.MapNC`

The **element maps must first be created** (see the previous section) and then subsequently assembled into lists:

```python
map_r1 = qbos.MapNC([map_r1_c1, map_r1_c2])
map_r2 = qbos.MapNC([map_r2_c1, map_r2_c2])
```

Remove the first occurrence of an element from a list:

```python
map_r1.remove(map_r1_c1)
```

Add an element to the front (index 0) of the list:

```python
map_r1.insert(0,map_r1_c1)
```

Add an element to the end of the list:

```python
map_r1.append(map_r1_c1)
```

Empty the list:

```python
map_r1.clear()
```

# 4.0 Assemble lists into a 2-D array, `qbos.VectorMapNC`

The **element lists must first be created** (see the previous section) and then subsequently assembled into the 2-D array:

```python
map_ = qbos.VectorMapNC([map_r1, map_r2])
```

Empty the 2-D array:

```python
map_.clear()
```