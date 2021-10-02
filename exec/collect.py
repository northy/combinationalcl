#!/usr/bin/env python
import subprocess as sp
import itertools
import platform
import signal

##### Definitions #####
dummy = False
P_start = 100
limit_P = 10000
K = 1000
#######################

def signal_handler(sig, frame):
    print('\nExiting...')
    exit(0)

signal.signal(signal.SIGINT, signal_handler)

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

limit_other = limit_P+1

def log_noexec(error,p,i,o) :
    with open("log_error.txt",'a') as f :
        f.writelines(['P: '+str(p)+', I: '+str(i)+', O: '+str(o)+"\n",str(error), '\n-----\n\n'])

def check_return(x,p,i,o) :
    global limit_other

    if x[1] :
        print(f"An error occured...",sep='\n')
        log_noexec(x[0],p,i,o)
        limit_other = max(i,o)

I_O_count = itertools.count(P_start//10,10)
P_count = itertools.count(P_start,100)

for I_O_max,P in zip(I_O_count,P_count) :
    if P>limit_P or limit_other<=limit_P : break
    for I,O in itertools.combinations_with_replacement(range(10,I_O_max+1,10), 2) :
        #this tries to prevent executions that would
        #most likely fail
        if O>=limit_other or I>=limit_other :
            log_noexec('skipping',P,I,O)
            if I!=O : log_noexec('skipping',P,O,I)
            continue
        check_return(doit(P,I,O), P,I,O)
        if I!=O :
            #if last execution failed, this will probably also fail
            if O>=limit_other or I>=limit_other :
                log_noexec('skipping',P,O,I)
                continue
            check_return(doit(P,O,I), P,O,I)
