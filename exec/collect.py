#!/usr/bin/env python
import subprocess as sp
import itertools
import platform

##### Definitions #####
dummy = False
limit_P = 10000
K = 10000
#######################

def doit(p,i,o) :
    print(f"Running with {p} ports, {i} inputs, {o} outputs (i+o+p={i+o+p})...")

    if dummy :
        if platform.system()=="Windows" : child = sp.Popen(['timeout', '2'], stdout = sp.PIPE, stderr=sp.PIPE)
        else : child = sp.Popen(['sleep', '2'], stdout = sp.PIPE, stderr=sp.PIPE)
    else :
        child = sp.Popen(['sh', 'run.sh', str(p), str(i), str(o), str(K)], stdout = sp.PIPE, stderr=sp.PIPE)
    
    errstr = child.communicate()[1]
    rc = child.returncode
    print("Done!")
    return errstr, rc

def check_return(x) :
    if x[1] : print(f"An error occured, aborting...",x[0],sep='\n'); exit(1)

I_O_count = itertools.count(10,10)
P_count = itertools.count(100,100)

for I_O_max,P in zip(I_O_count,P_count) :
    if P>limit_P : break
    for I,O in itertools.combinations_with_replacement(range(10,I_O_max+1,10), 2) :
        check_return(doit(P,I,O))
        if I!=O : check_return(doit(P,O,I))
