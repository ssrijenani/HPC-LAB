# Performance Analysis of OpenMP Scheduling Policies

## Objective and workload

Compare explicit `schedule(static)`, `schedule(dynamic, 8)` and `schedule(guided, 8)`
using the same loop, workload and four requested threads. Every policy writes exactly
one output value per iteration; there are no shared accumulators or data races.

There are 8,000 iterations. The first quarter performs 60,000 integer recurrence steps
per iteration, while the remaining iterations perform 1,000. This intentionally clusters
heavy work at the start. Defined unsigned wraparound keeps the output deterministic.
The serial reference is computed once before timing; every parallel output element must
match it exactly. The printed checksum is an additional readable reference.

| Policy | Work distribution | Expected tradeoff |
| --- | --- | --- |
| static | Fixed contiguous blocks | Low scheduling overhead, but unequal work for this input |
| dynamic,8 | Threads request eight iterations at a time | Better load balance, with dispatch overhead |
| guided,8 | Shrinking chunks, minimum eight iterations | Fewer dispatches, but early large chunks can contain heavy work |

## Requirements and commands

Linux GCC with its OpenMP runtime; from this folder:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -fopenmp openmp.c -o openmp
OMP_DYNAMIC=FALSE OMP_PROC_BIND=TRUE OMP_PLACES=cores ./openmp 4
./openmp --test
./openmp 1
```

The optional argument is a thread count from 1..64; the default is four. `--test` runs
a quick 32-iteration comparison. The program disables dynamic team sizing, warms the
thread team and checks that the actual count equals the requested count. Every timed
parallel region explicitly requests the same count. CI records available CPUs and OpenMP
settings so oversubscription can be recognized.

## Measurement and correctness

`omp_get_wtime()` measures wall-clock seconds including parallel-region and scheduling overhead.
There are three trials per policy, rotating the order. Output initialization, serial-reference
generation and element-by-element verification are excluded from timing. Mean and minimum
are printed. Any mismatch exits with failure. CI also checks a single-thread run, bad arguments
and sanitizer builds.

## Conclusion and limitations

Dynamic scheduling is expected to help this deliberately uneven workload because free threads
can take more chunks. Guided scheduling reduces dispatches but can give a large early heavy
chunk to one thread. Static scheduling can be faster on uniform workloads. The actual winner
below is determined by measured mean time, not assumed from theory.

These results apply to this workload, chunk size and environment. Shared runner contention,
available physical cores, frequency scaling, affinity and OpenMP runtime overhead can alter the
ranking. A one-thread run checks correctness but does not demonstrate parallel speedup; a
four-thread run on fewer than four available cores must be interpreted as oversubscribed.

<!-- ACTUAL_RESULTS -->

## Actual Ubuntu execution results

[Verified workflow run](https://github.com/ssrijenani/Performance-Analysis-Projects/actions/runs/41972357995). Environment: [environment.txt](../environment.txt).

Measured with GCC `-O2`; three trials; seconds. These are actual captured measurements.

| Method | Mean seconds | Minimum seconds | Correct |
| --- | ---: | ---: | --- |
| static | 0.187350475 | 0.187071665 | PASS |
| dynamic,8 | 0.049748506 | 0.049666937 | PASS |
| guided,8 | 0.187021253 | 0.186973621 | PASS |

### Captured program output

```text
Iterations=8000; threads requested=4; actual=4; trials=3
Serial reference checksum=17319206870328
Policy          | Mean seconds | Minimum seconds | Correct
static          | 0.187350475  | 0.187071665     | PASS
dynamic,8       | 0.049748506  | 0.049666937     | PASS
guided,8        | 0.187021253  | 0.186973621     | PASS
Fastest measured policy: dynamic,8
Verification: all policies match the serial reference at EVERY iteration.
```

Full checks: [tests.txt](tests.txt). All commands completed successfully, including sanitizer checks.

### Measured conclusion

**dynamic,8** had the lowest mean elapsed time in this environment. Small requested chunks let idle threads take more of the expensive first quarter, improving balance despite dispatch overhead. This explanation is an interpretation of the measurements, not a separate measurement of scheduling overhead.
