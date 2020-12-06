#!/usr/bin/env python
import subprocess as sp

K = 10000

def doit(p,i,o) :
    child = sp.Popen(['sh', 'run.sh', str(p), str(i), str(o), str(K)], stdout = sp.PIPE, stderr=sp.PIPE)
    print(f"Running with {p} ports, {i} inputs, {o} outputs (i+o+p={i+o+p})...")
    errstr = child.communicate()[1]
    rc = child.returncode
    return errstr, rc

errstr, rc = doit(1,1,1)
if (rc) :
    print(errstr)
    exit(1)

p,i,o = 6,2,2
for it_ in range(10000) :
    errstr, rc = doit(p,i,o)
    if (rc) :
        print("An error has occured:")
        print(errstr)
        exit(1)
    p+=6; i+=2; o+=2

