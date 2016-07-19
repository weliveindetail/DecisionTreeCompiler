# EvalTreeJit

Current speedups reach from 4.5 to 5.5 compared to regular evaluation using [Flattened Tree Method](http://tullo.ch/articles/decision-tree-evaluation/#flattened-tree-method). Actively working on further improvements to achieve even more! Here's some sample benchmarks:

```
./EvalTreeJit
Building decision tree with depth 16 and cache it in file cache/_td16_dsf100.t
Generating 1 evaluators for 65535 nodes and cache it in file cache/_td16_dsf100_cfd16_cfsd2.o
Composing... took 8253 seconds
Compiling... took 2413 seconds
Collecting...

Run on (4 X 1000 MHz CPU s)
2016-07-18 23:32:06
Benchmark                      Time           CPU Iterations
------------------------------------------------------------
BM_RegularEvaluation         664 ns        663 ns    1189384
BM_CompiledEvaluation        131 ns        131 ns    5099848


./EvalTreeJit
Loading decision tree with depth 16 from file cache/_td16_dsf100.t
Loading 1 evaluators for 65535 nodes from file cache/_td16_dsf100_cfd16_cfsd2.o

Run on (4 X 1000 MHz CPU s)
2016-07-18 23:34:53
Benchmark                      Time           CPU Iterations
------------------------------------------------------------
BM_RegularEvaluation         955 ns        724 ns     968885
BM_CompiledEvaluation        201 ns        185 ns    5261330


./EvalTreeJit
Loading decision tree with depth 16 from file cache/_td16_dsf100.t
Loading 1 evaluators for 65535 nodes from file cache/_td16_dsf100_cfd16_cfsd2.o

Run on (4 X 1000 MHz CPU s)
2016-07-18 23:35:35
Benchmark                      Time           CPU Iterations
------------------------------------------------------------
BM_RegularEvaluation         845 ns        710 ns    1064024
BM_CompiledEvaluation        161 ns        156 ns    4703011

```

# Todo
* add command line interface
* fix failing optimization passes (currently inactive)
* SIMD optimize node evaluation groups
* reduce compile times
