# Run the projects

## Easiest full run on this Mac: GitHub Actions

Apple Clang can run sorting locally, but the required GCC OpenMP and GNU gprof runs use Ubuntu.

1. Open https://github.com/ssrijenani/HPC-LAB/actions/workflows/benchmark.yml while signed in.
2. Click **Run workflow**, leave branch **main**, and click the green **Run workflow** button.
3. Open the new run, then click **benchmark**.
4. Expand **Compile and test sorting** and **Compile and test OpenMP** to see the live output.
5. A green check means both programs and their checks passed. The workflow saves fresh results and READMEs in the repository.
6. In VS Code choose **Terminal > New Terminal** and run `git pull --ff-only` from the HPC-LAB folder to download those reports.

The workflow also checks the source and these execution instructions when they change.

## Run sorting directly in VS Code on this Mac

Choose **Terminal > New Terminal**. From the HPC-LAB folder:

```sh
cd Performance-Analysis-Projects/Sorting-Algorithms
clang -std=c11 -Wall -Wextra -O2 sorting.c -o sorting
./sorting
```

Every algorithm should print `PASS`. Times vary between runs. To run only the quick correctness checks: `./sorting --test`.

## Full manual commands on Ubuntu or Linux

From the HPC-LAB folder, with GCC and binutils installed:

```sh
cd Performance-Analysis-Projects/Sorting-Algorithms
gcc -std=c11 -Wall -Wextra -O2 sorting.c -o sorting
./sorting
gcc -std=c11 -Wall -Wextra -O0 -g -pg -fno-inline sorting.c -o sorting_pg
./sorting_pg
gprof ./sorting_pg gmon.out > gprof-report.txt
head -20 gprof-report.txt
cd ../OpenMP-Scheduling
gcc -std=c11 -Wall -Wextra -O2 -fopenmp openmp.c -o openmp
OMP_DYNAMIC=FALSE OMP_PROC_BIND=TRUE OMP_PLACES=cores ./openmp 4
```

`-O2` timings and `-O0 -pg` profiling results come from separate executables. All three schedules should print `PASS` and the same serial reference is checked at every iteration.
