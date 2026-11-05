# Profiling and Optimization of Sorting Algorithms

## Objective and implementation

Implement Bubble Sort, Selection Sort and Merge Sort in simple C, verify their results,
compare timings, and identify computational hotspots with GNU gprof.

| Algorithm | Typical time | Extra space | Implementation detail |
| --- | --- | --- | --- |
| Bubble Sort | O(n²), O(n) best case | O(1) | Adjacent swaps; stop when a pass makes no swaps |
| Selection Sort | O(n²) | O(1) | Select the smallest remaining element |
| Merge Sort | O(n log n) | O(n) | Recursively sort halves and merge; reuse one scratch array |

All algorithms receive copies of the exact same 18,000 deterministic integers (seed 42).
There are three trials with rotated order. Allocation, input copying, reference sorting and
verification are outside the timer. Every output element must equal an independent `qsort`
reference, which verifies sorted order AND preservation of the original values.

## Requirements and commands

Use Linux with GCC and GNU binutils. From this folder:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 sorting.c -o sorting
./sorting
./sorting --test
./sorting 1000
gcc -std=c11 -Wall -Wextra -Wpedantic -O0 -g -pg -fno-inline sorting.c -o sorting_pg
./sorting_pg
gprof ./sorting_pg gmon.out > gprof-report.txt
```

The `-pg` flag is present during both compilation and linking (one command). Running the
instrumented executable creates `gmon.out` in the current directory. `-O0 -fno-inline` keeps
functions visible for teaching; the uninstrumented `-O2` executable supplies the comparison timings.
The optional size is bounded to 1..50,000 to keep quadratic runs manageable.

## Testing and interpretation

Built-in tests cover empty input, a singleton, sorted input, reverse input, duplicate values,
negative values and a reversed pair: seven cases for each of three algorithms. The benchmark
also checks all nine complete sorted arrays. CI additionally runs sanitizers and invalid-input checks.

Bubble and Selection Sort repeatedly scan large portions of the array; their work grows
quadratically. Bubble Sort can add many adjacent swaps, whereas Selection Sort only swaps
once per outer pass. Merge Sort does fewer comparisons asymptotically, while allocating a
scratch buffer once outside timing. Bubble's early-exit optimization helps already sorted input.

In gprof, **self seconds** measure time sampled inside a function; **total seconds** in the call
graph include its callees. Use the flat profile to identify the largest self-time hotspots and
the call graph to see Merge Sort's recursive calls and merge cost. The full saved report is
the evidence; short functions may receive zero samples at gprof's sampling interval.

## Limitations

Only one random distribution and teaching-scale sizes are benchmarked. Three repeats cannot
remove shared-machine noise. Instrumentation and `-O0` change timings, so profiling percentages
are interpreted separately from the `-O2` comparison. These results do not establish a universal
ranking across compilers, distributions or hardware. Full raw reports are kept with this project.

<!-- ACTUAL_RESULTS -->

## Actual Ubuntu execution results

[Verified workflow run](https://github.com/ssrijenani/HPC-LAB/actions/runs/41973064646). Environment: [environment.txt](../environment.txt).

Measured with GCC `-O2`; three trials; seconds. These are actual captured measurements.

| Method | Mean seconds | Minimum seconds | Correct |
| --- | ---: | ---: | --- |
| Bubble Sort | 0.423394112 | 0.417921337 | PASS |
| Selection Sort | 0.046536778 | 0.046338247 | PASS |
| Merge Sort | 0.001082753 | 0.001069614 | PASS |

### Captured program output

```text
Self-tests: PASS (7 cases x 3 algorithms: empty, singleton, sorted, reverse, duplicates, negatives, pair)
Input: 18000 integers; seed=42; trials=3; identical input for every run
Algorithm       | Mean seconds | Minimum seconds | Correct
Bubble Sort     | 0.423394112  | 0.417921337     | PASS
Selection Sort  | 0.046536778  | 0.046338247     | PASS
Merge Sort      | 0.001082753  | 0.001069614     | PASS
Slowest measured algorithm: Bubble Sort
Verification: every output element equals the independently qsort-sorted reference.
```

Full checks: [tests.txt](tests.txt). All commands completed successfully, including sanitizer checks.

### Hotspot analysis and conclusion

The slowest optimized algorithm in this run was **Bubble Sort**. The quadratic sorts scan repeatedly, while Merge Sort divides and merges in O(n log n). Bubble Sort also swaps adjacent inversions.

Actual gprof flat-profile hotspots (`-O0 -pg`, separate run):

| Function | Self time share | Self seconds |
| --- | ---: | ---: |
| `bubble_sort` | 63.92% | 0.62 |
| `selection_sort` | 35.05% | 0.34 |
| `merge` | 1.03% | 0.01 |

These shares describe sampled self time in the instrumented run, not the optimized timing table. Recursive Merge Sort delegates work to `merge`; consult the call graph for inclusive costs. Sampling may miss very short functions. [Complete gprof report](gprof-report.txt); [instrumented output](profile-output.txt).
