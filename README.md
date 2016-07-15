# EvalTreeJit

Current speedups reach from 2.5 to 3.5 compared to regular evaluation using [Flattened Tree Method](http://tullo.ch/articles/decision-tree-evaluation/#flattened-tree-method). Actively working on further improvements to achieve an average speedup of 4. Here's some sample benchmarks:

```
./EvalTreeJit
Building decision tree with depth 15 and cache it in file cache/_td15_dsf100.t
Generating 1 evaluators for 32767 nodes and cache it in file cache/_td15_dsf100_cfd15_cfsd3.o
Composing... took 2091 seconds
Compiling... took 68 seconds
Collecting...
Run on (4 X 1000 MHz CPU s)
2016-07-15 02:49:01
Benchmark                      Time           CPU Iterations
------------------------------------------------------------
BM_RegularEvaluation         683 ns        681 ns    1340714
BM_CompiledEvaluation        227 ns        226 ns    3391604

./EvalTreeJit
Loading decision tree with depth 15 from file cache/_td15_dsf100.t
Loading 1 evaluators for 32767 nodes from file cache/_td15_dsf100_cfd15_cfsd3.o

Run on (4 X 1000 MHz CPU s)
2016-07-15 03:03:48
Benchmark                      Time           CPU Iterations
------------------------------------------------------------
BM_RegularEvaluation         770 ns        656 ns    1107893
BM_CompiledEvaluation        307 ns        249 ns    2836741

./EvalTreeJit
Loading decision tree with depth 15 from file cache/_td15_dsf100.t
Loading 1 evaluators for 32767 nodes from file cache/_td15_dsf100_cfd15_cfsd3.o

Run on (4 X 1000 MHz CPU s)
2016-07-15 03:04:46
Benchmark                      Time           CPU Iterations
------------------------------------------------------------
BM_RegularEvaluation         916 ns        721 ns    1097471
BM_CompiledEvaluation        257 ns        220 ns    2951656
```

# Todo
* add command line interface
* fix failing optimization passes (currently inactive)
* SIMD optimize node evaluation groups
* reduce compile times
