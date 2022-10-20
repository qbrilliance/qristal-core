# Usage/examples of qbos.MapND type

# 1.0 Background

A data type, `qbos.ND` is provided in qbOS for storing **maps**.  The key-value pairs for this type use integer keys.  The associated pair value is a double-precision real value.   These maps are useful as storage for **sparse** vectors of doubles.

# 2.0 Creating elements (of type `qbos.ND` that map integer → double precision real)

```python
import qbos

map_r1_c1 = qbos.ND()  # A map
map_r1_c2 = qbos.ND()  # A second map

map_r2_c1 = qbos.ND()  # A third map
map_r2_c2 = qbos.ND()  # A fourth map
```

Populate the maps:

```python
map_r1_c1[0] = 1.23  # map_r1_c1: 0 -> 1.23
map_r1_c1[1] = -0.5  # map_r1_c1: 1 -> -0.5
map_r1_c1[2] = 0.2 # map_r1_c1: 2 -> 0.2

map_r1_c2[0] = 1.1  # map_r1_c2: 0 -> 1.1
map_r1_c2[1] = -0.8  # map_r1_c2: 1 -> -0.8
map_r1_c2[2] = 0.1 # map_r1_c2: 2 -> 0.1

map_r2_c1[0] = 1.31  # map_r1_c1: 0 -> 1.31

map_r2_c2[0] = 2.1 # map_r2_c2: 0 -> 2.1
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

# 3.0 Assemble elements into a list, `qbos.MapND`

The **element maps must first be created** (see the previous section) and then subsequently assembled into lists:

```python
map_r1 = qbos.MapND([map_r1_c1, map_r1_c2])
map_r2 = qbos.MapND([map_r2_c1, map_r2_c2])
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

# 4.0 Assemble lists into a 2-D array, `qbos.VectorMapND`

The **element lists must first be created** (see the previous section) and then subsequently assembled into the 2-D array:

```python
map_ = qbos.VectorMapND([map_r1, map_r2])
```

Empty the 2-D array:

```python
map_.clear()
```