# EvalTreeJit

```
$ ./EvalTreeJit
Building decision tree with depth 20 and cache it in file cache/_td20_dsf100.t
Generating 1025 evaluators for 1048575 nodes and cache it in file cache/_td20_dsf100_cfd10.o
Compiling... took 128 seconds

Benchmarking: 1000 runs with 100 features
Average evaluation time regular: 4006.025879ns
Average evaluation time compiled: 1025.645020ns

$ ./EvalTreeJit
Loading decision tree with depth 20 from file cache/_td20_dsf100.t
Loading 1025 evaluators for 1048575 nodes from file cache/_td20_dsf100_cfd10.o

Benchmarking: 1000 runs with 100 features
Average evaluation time regular: 4597.800781ns
Average evaluation time compiled: 1265.265015ns

$ ./EvalTreeJit
Loading decision tree with depth 20 from file cache/_td20_dsf100.t
Generating 32769 evaluators for 1048575 nodes and cache it in file cache/_td20_dsf100_cfd15.o
Compiling... took 400 seconds

Benchmarking: 1000 runs with 100 features
Average evaluation time regular: 3656.186035ns
Average evaluation time compiled: 1079.397949ns

$ ./EvalTreeJit
Loading decision tree with depth 20 from file cache/_td20_dsf100.t
Generating 1 evaluators for 1048575 nodes and cache it in file cache/_td20_dsf100_cfd20.o
Compiling... (never finished)

$ ls -lh cache
-rw-r--r--  1  151M 15 Jun 21:15 _td20_dsf100.t
-rw-r--r--  1   14M 15 Jun 21:18 _td20_dsf100_cfd10.o
-rw-r--r--  1   22M 15 Jun 23:49 _td20_dsf100_cfd15.o
```

# Todo
* refactoring
* command line interface
* improve compile times
* optimize runtime-compiled code
