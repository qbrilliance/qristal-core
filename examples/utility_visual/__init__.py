from importlib import import_module

file_split = str.split(__file__,"/")
name_split = str.split(__name__,".")
root = file_split.index(name_split[0])
address_root = str.join(".",file_split[root:-1])
#
qbos_vis_address = address_root + "." + "qbos_vis"
globals()["qbos_vis"] = import_module(qbos_vis_address)
