__all__ = ["quantum_utility","tomography"]
from importlib import import_module
from pyQuirk import Quirk
quirk = Quirk(height=280)
file_split = str.split(__file__,"/")
name_split = str.split(__name__,".")
root = file_split.index(name_split[0])
address_root = str.join(".",file_split[root:-1])
#
qutil_address = address_root + "." + "quantum_utility"
globals()["qbos_vis"] = import_module(qutil_address)

qtm_address = address_root + "." + "tomography"
globals()["qbos_vis"] = import_module(qtm_address)