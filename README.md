# DecisionTreeCompiler

A research project on domain-specific optimizations for compiled decision tree evaluators. Please find a full review in one of my upcoming blog posts.

Quick links: [Objective](#objective), [Background](#background), [The Idea](#idea), [Conclusion](#conclusion)

## TL;DR

Preliminary results:
* runtime compilation gives maximum efficiency and preserves high fexibility
* jump tables for leaf-subtrees can be more efficient then if-then-else
* different data-set sizes affect runtimes insignificantly
* otherwise if-then-else is here to stay (for now), as in general the benefits of subtree switches are too little to regain the ground they lose with additional loads

Here is the latest benchmark results for compiled evaluators with different domain-specific optimizations. Please find exact numbers [here](https://github.com/weliveindetail/DecisionTreeCompiler/blob/master/docs/2016-09-benchmarks-a8b55ac0.txt).
![2016-09-benchmarks-5-features](https://github.com/weliveindetail/DecisionTreeCompiler/blob/master/docs/2016-09-benchmarks-a8b55ac0-5-features.png)

<a name="objective"/>
## Objective

Decision trees have a very specific structure and a well-defined evaluation process. I want to elaborate strategies that utilize this knowledge to gain runtime performance and make most efficient use of available hardware resources. The solution should be close to the simple interpretive approach in terms of generality, maintenance and manual interaction.

<a name="background"/>
## Background

### The General Evaluation Mechanism

Evaluating a decision tree is a simple task: Evaluate a node to select a child and continue with that one. Starting at the root node, we will bubble through the tree level by level and eventually reach a leaf node, which contains the information we asked for. Evaluating a single node is simple too: Load a distinct value from the input data set and compare it to the node's bias. The comparison result selects the child node to continue with.

### Complexity Analysis

A single node evaluation involves to following operations:

1. select the input value from the data set
2. load the input value
3. load the node's bias value
4. compare the two values
5. select a child node based on the result
6. invoke the child node evaluation

This takes constant time. Respectively the asymptotic complexity of any decision tree evaluation method is at best linear to the depth of the desired leaf node. The constant factor of the slope's inclination, however, may be target for optimization.

### Common approaches

Interpretive methods load a tree at run-time and execute its directions for any input data-set like an interpreter. Implementations can be very general, on the one hand, as a single one may work for arbitrary decision trees. On the other hand, they lack efficiency as all the above mentioned steps for evaluating a single node happen at run-time.

Compiled methods load a tree at compile-time and generate an implementation for it. The implementation runs the evaluation process for the specific tree and any input data-set. Generality can be considered zero, as implementations are not applicable to other trees. However, this opens the door for optimizations that wouldn't be possible otherwise. An example is data-inlining: Instead of loading the data for each tree node at run-time, it can be injected in the code as immediate values at compile-time. In the above mentioned steps for evaluating a single node, this eliminates steps 1 (we know which input value to use) and 3 (the node's bias is inlined) and simplifies steps 5 and 6 (we know the possible child nodes and branch to their evaluators directly). 

### The conventional compiled tree method in detail

Common compiled tree methods generate high-level code, compile it ahead of time with a general-purpose compiler and link it dynamically to the consuming application. Here is a simple example tree:

![tree-2level](https://github.com/weliveindetail/DecisionTreeCompiler/blob/master/docs/tree-2level.png)

There are 3 interior nodes for decisions and 4 leaf nodes representing the results, in short, a _perfect binary decision tree of depth 2_. The code generated for this example may look like that:

```
int evaluate(float *data_set) {
    bool N0_result = (data_set[N0_feature_index] > N0_bias);
    if (N0_result) {
        bool N2_result = (data_set[N2_feature_index] > N2_bias);
        return (N2_result) ? R3_value : R2_value;
    } else {
        bool N1_result = (data_set[N1_feature_index] > N1_bias);
        return (N1_result) ? R1_value : R0_value;
    }
}
```

Note that the values of `Nx_feature_index`, `Nx_bias` and `Rx_value` are compile-time constants. This is considered a number of times more efficient that interpretive methods. Implementations like [sklearn-compiledtrees](https://github.com/ajtulloch/sklearn-compiledtrees) state speedups of 5x to 8x. 

<a name="idea"/>
## The Idea

The run-time cost of the above code is dominated by two types of operations:
* **Loads from the data-set** to read the actual value of the feature compared by the node: In general a load operation is very expensive as it may read from main memory directly. Fortunately in this case the entire data-set should fit into L1 cache and we only read from it (no cache sync required). Thanks to modern CPU's instruction pipelining, loads only cause medium latency as they are prepared in advance.
* **Conditional jumps** to the respective child node evaluation: Conditional jumps may cause problems with instruction pipelining. As the subsequent instruction is not known before the condition is evaluated, execution cannot be prepared accordingly. [Branch prediction](http://igoro.com/archive/fast-and-slow-if-statements-branch-prediction-in-modern-processors/) may reduce the effect.

In the evaluator code, however, loads and conditional jumps are perfectly interleaved and build on the results of each other. There seem to be no straightforward opportunities for further optimization.

In order to get even better performance, we could transform the code in a way the C compiler is not allowed to: Instead of generating nested if-statements comparing a feature value each, we could evaluate all nodes of a subtree at once and use a switch statement to do a single conditional jump instead of multiple ones. This way we pay the cost of additional node evaluations to reduce the number of conditional jumps and create bigger blocks of unconditional operations. For illustration, the above example rewritten in pseudo-code:

```
int evaluate(float *data_set) {
    bool N0_result = (data_set[N0_feature_index] > N0_bias);
    bool N1_result = (data_set[N1_feature_index] > N1_bias);
    bool N2_result = (data_set[N2_feature_index] > N2_bias);

    switch {
        case !N0_result && !N1_result: return R0_value;
        case !N0_result &&  N1_result: return R1_value;
        case  N0_result && !N2_result: return R2_value;
        case  N0_result &&  N2_result: return R3_value;
    }
}
```

This obviously won't scale to full trees! (As number nodes in a binary tree grows exponentially with the number of levels.) Instead it's about finding a sweet spot where subtree switches are more efficient that conventional the if-then-else. The evaluator for a full tree would then consist of nested switches instead of nested if-then-elses. Additionally it may also act as a door-opener for other optimizations:
* nodes may be evaluated in parallel exploiting the CPU's SIMD features
* leaf-subtrees may be compiled to jump-tables with no conditional branching
* if multiple nodes in the subtree compare the same feature, it may save a few loads

<a name="conclusion"/>
## Conclusion

Here are the insights I gained from implementing and benchmarking different combinations of the ideas. Although I consider the main objective rejected by the benchmark results, there are a few positive and interesting findings:
* runtime compilation gives us the best of both worlds, maximum efficiency and high flexibility (with a one-time compilation overhead)
* increasing data-set sizes from 5 to 10000 features has not noticable effect on the runtime of compiled evaluators (maybe due to sufficiently large L1 cache)
* jump tables for leaf-subtrees are indeed more efficient then their if-then-else equivalents

Apart from that, if-then-else is here to stay. In general the benefits of subtree switches are too little to regain the ground they lose with additional loads. They greatly reduce the number of conditional jumps, but they benefit much less from branch prediction.

SIMD-parallelization outweighs its initial overhead only for 7 or more nodes in parallel. At this point we switch over 3 levels and produce 5 additional loads. This causes more overhead than the parallelization saves. Subtrees with more than 3 levels are always less efficient.
