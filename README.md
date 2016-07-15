# EvalTreeJit

```
$ ./EvalTreeJit
Building decision tree with depth 15 and cache it in file cache/_td15_dsf100.t
Generating 1 evaluators for 32767 nodes and cache it in file cache/_td15_dsf100_cfd15_cfsd3.o

Run on (4 X 1000 MHz CPU s)
2016-07-15 01:55:19
Benchmark                      Time           CPU Iterations
------------------------------------------------------------
BM_RegularEvaluation         813 ns        680 ns    1038514
BM_CompiledEvaluation        377 ns        316 ns    2353558

$ ./EvalTreeJit
Loading decision tree with depth 15 from file cache/_td15_dsf100.t
Loading 1 evaluators for 32767 nodes from file cache/_td15_dsf100_cfd15_cfsd3.o

Run on (4 X 1000 MHz CPU s)
2016-07-15 02:02:18
Benchmark                      Time           CPU Iterations
------------------------------------------------------------
BM_RegularEvaluation         773 ns        658 ns    1086771
BM_CompiledEvaluation        271 ns        251 ns    2612242
```

# Todo
* command line interface
* fix failing optimization passes (currently inactive)
* SIMD optimize node evaluation groups
