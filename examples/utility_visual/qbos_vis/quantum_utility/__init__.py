from importlib import import_module
file_split = str.split(__file__,"/")
name_split = str.split(__name__,".")
root = file_split.index(name_split[0])
address_root = str.join(".",file_split[root:-1])

p_address = address_root + "." + "plot"
globals()["advice"] = import_module(p_address)