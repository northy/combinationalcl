# CombinationalCL

Gate-level digital circuits generator and simulator.

## Combinational circuit generator

Provided the following input arguments:

* Input count (**I**): The amount of input logic wires to the circuit.
* Port count (**P**): The amount of logic gates in the circuit, each of them corresponding to a intermediate signal (net) in the circuit.
* Output count (**O**): The amount of input logic wires to the circuit.
* Events count (**K**): The number of events to be simulated for the circuit.

This tool generates a logic circuit with **I** inputs, **O** outputs and **P** logic gates with random inputs and boolean logic operation. The current subset of logic operations is: AND, OR, XOR, NOT.

This tool then generates an OpenCL kernel with the correspondent operations, and a VCD file with **K** input dumps to be simulated.

Execute with: ```cargo run <P> <I> <O> <K>```

Example execution: ```cargo run 600 20 20 1000```

This tool is currently under ```rust/```.

## OpenCL host platform

Provided the arguments in the last tool and the circuit's kernel and VCD, this tool is used to simulate the generated circuit and stimuli with the OpenCL platform.

The output is a VCD file with the simulated output signals and metrics about the execution. Currently the recorded metrics are:

* I
* O
* P
* K
* CPU execution time (ns)
* GPU execution time (ns)

Execute with: ```./CombinationaCL <ports count> <input count> <output count> <kernel file> <vcd file>```

Example execution: ```./CombinationaCL 600 20 20 out.cl out.vcd```

This tool is currently under ```cpp/```.

## Data colector

This tool is meant to collect data by executing both of the previous tools with a single command.

Compile both tools: ```./compile.sh```

Single command execution:: ```./run.sh <ports count> <inputs count> <outputs count> <events count>```

Data collection execution (change parameters in code): ```./collect.py```

A copy of `log.def.csv` will be created as `log.csv`, and following executions will append metrics there.

This tool is currently under ```exec/```.

### Running in EC2

You can use:

```sh
screen -S data_collect -dm bash -c "./ec2_collect.sh"
```

To run the data collector in an EC2 environment.


You can check the progress with:

```sh
screen -S data_collect -r
# (CTRL+a, d to leave)
```

To stop the data collector simply run:

```sh
killall python3
```
