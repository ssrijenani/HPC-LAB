# Performance Analysis Projects

Two beginner-friendly C projects: sorting algorithm profiling and OpenMP scheduling analysis.

| Folder | Program | Purpose |
| --- | --- | --- |
| [Sorting-Algorithms](Sorting-Algorithms/) | `sorting.c` | Bubble, Selection and Merge Sort; timing, correctness and GNU gprof |
| [OpenMP-Scheduling](OpenMP-Scheduling/) | `openmp.c` | Static, dynamic and guided schedules on an uneven workload |

## Quick start (Ubuntu / Linux)

Requirements: GCC, GNU binutils (`gprof`), and GCC's OpenMP runtime. On Ubuntu, if missing:

```sh
sudo apt-get update
sudo apt-get install -y build-essential binutils
```

```sh
cd Sorting-Algorithms
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 sorting.c -o sorting
./sorting
gcc -std=c11 -Wall -Wextra -Wpedantic -O0 -g -pg -fno-inline sorting.c -o sorting_pg
./sorting_pg
gprof ./sorting_pg gmon.out > gprof-report.txt
cd ../OpenMP-Scheduling
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -fopenmp openmp.c -o openmp
OMP_DYNAMIC=FALSE OMP_PROC_BIND=TRUE OMP_PLACES=cores ./openmp 4
```

The optimized timing executable is separate from the instrumented profiling executable.
Never compare a profiled Bubble Sort run against an unprofiled Merge Sort run.

## Reproducible execution

The included GitHub Actions workflow compiles both programs on Ubuntu, runs correctness checks,
AddressSanitizer/UndefinedBehaviorSanitizer checks, full benchmarks and GNU gprof. A successful
run commits its actual text reports and updates each README with the captured output. It fails
on compilation errors, incorrect results or sanitizer findings. Generated documentation commits
do not trigger another benchmark.

Timing is environment-specific. Three trials rotate policy/algorithm order. Mean and minimum
times are reported; copying inputs, serial references and result verification are excluded from
timed sections. Sorting uses monotonic wall time; OpenMP uses `omp_get_wtime()`.

## Environment and limitations

Development host: Apple Silicon macOS with Apple Clang. The command named `gcc` on this Mac
is Apple Clang and rejects `-fopenmp`; GNU gprof is absent. Therefore the required GCC,
OpenMP and gprof evidence is generated on the Ubuntu GitHub Actions runner, not claimed
as a native macOS run. The source needs no paid services or third-party C libraries.

GitHub's public-repository hosted runner is used for testing. Shared runner load, CPU frequency,
thread placement and profiling overhead affect measurements. These are small teaching
experiments, not universal hardware rankings. See the per-project READMEs and saved reports.
