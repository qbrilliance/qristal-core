# Copyright (c) 2022 Quantum Brilliance Pty Ltd
import xacc
import re
from IPython.display import display, Markdown, Latex
def print(in_oqm) :
    mstaq = xacc.getCompiler('staq')
    mcc = mstaq.compile(in_oqm)
    circuit_xasm_in = (mcc.getComposite('QBCIRCUIT')).toString()
    cols=(re.sub(r"([A-Za-z0-9,\-\.\(\)]+)[ ]*q([\d]+).*", r"\2 ",circuit_xasm_in)).split()
    icols = [int(elc) for elc in cols]
    istr = circuit_xasm_in[0:].splitlines()
    fmd=['|'+'|'*int(elc) + els + '|' for elc,els in zip(cols, istr)]
    l1='| ' + ' | '.join(['q'+str(ii) for ii in range(0,max(icols)+1)]) + ' |'
    l2 = '| ' + ' | '.join(['---' for ii in range(0,max(icols)+1)]) + ' |'
    display(Markdown(l1 + '\n' + l2 + '\n' + '\n'.join(fmd)))

