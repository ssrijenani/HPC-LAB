#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <omp.h>

/* The first quarter is much heavier, so contiguous static blocks are unequal.
   Unsigned arithmetic has defined wraparound and gives reproducible results. */
static uint32_t work(int i, int n) {
    int steps = (i < n / 4) ? 60000 : 1000;
    uint32_t value = (uint32_t)i + 1;
    for (int j = 0; j < steps; ++j)
        value = (value * UINT32_C(1664525) + UINT32_C(1013904223)) ^ (value >> 13);
    return value;
}

/* Explicit pragmas make each scheduling policy easy to find and change. */
static double run(int policy, uint32_t output[], int n, int threads) {
    double start = omp_get_wtime();
    if (policy == 0) {
        #pragma omp parallel for num_threads(threads) schedule(static)
        for (int i = 0; i < n; ++i) output[i] = work(i, n);
    } else if (policy == 1) {
        #pragma omp parallel for num_threads(threads) schedule(dynamic, 8)
        for (int i = 0; i < n; ++i) output[i] = work(i, n);
    } else {
        #pragma omp parallel for num_threads(threads) schedule(guided, 8)
        for (int i = 0; i < n; ++i) output[i] = work(i, n);
    }
    return omp_get_wtime() - start;
}

int main(int argc, char *argv[]) {
    int n = 8000, threads = 4, trials = 3;
    int test = (argc == 2 && strcmp(argv[1], "--test") == 0);
    if (test) n = 32;
    else if (argc == 2) {
        char *end;
        long value = strtol(argv[1], &end, 10);
        if (*end || value < 1 || value > 64) { fputs("Threads must be 1..64\n", stderr); return 1; }
        threads = (int)value;
    } else if (argc > 2) { fputs("Usage: ./openmp [--test | threads 1..64]\n", stderr); return 1; }
    omp_set_dynamic(0);
    int actual = 0;
    /* Warm the team before timing and check requested parallelism is available. */
    #pragma omp parallel num_threads(threads)
    {
        #pragma omp single
        actual = omp_get_num_threads();
    }
    if (actual != threads) { fprintf(stderr, "Requested %d threads; got %d\n", threads, actual); return 1; }
    uint32_t *expected = malloc((size_t)n * sizeof *expected);
    uint32_t *output = malloc((size_t)n * sizeof *output);
    if (!expected || !output) { fputs("Allocation failed\n", stderr); free(expected); free(output); return 1; }
    uint64_t checksum = 0;
    for (int i = 0; i < n; ++i) { expected[i] = work(i, n); checksum += expected[i]; }
    double total[3] = {0}, minimum[3] = {1e100,1e100,1e100};
    const char *names[] = {"static", "dynamic,8", "guided,8"};
    for (int trial = 0; trial < trials; ++trial) {
        for (int offset = 0; offset < 3; ++offset) {
            int p = (trial + offset) % 3;
            memset(output, 0, (size_t)n * sizeof *output);
            double elapsed = run(p, output, n, threads);
            for (int i = 0; i < n; ++i) {
                if (output[i] != expected[i]) {
                    fprintf(stderr, "FAILED: %s at iteration %d\n", names[p], i);
                    free(expected); free(output); return 1;
                }
            }
            total[p] += elapsed;
            if (elapsed < minimum[p]) minimum[p] = elapsed;
        }
    }
    printf("Iterations=%d; threads requested=%d; actual=%d; trials=%d\n", n, threads, actual, trials);
    printf("Serial reference checksum=%llu\n", (unsigned long long)checksum);
    puts("Policy          | Mean seconds | Minimum seconds | Correct");
    for (int p = 0; p < 3; ++p)
        printf("%-15s | %.9f  | %.9f     | PASS\n", names[p], total[p] / trials, minimum[p]);
    int fastest = 0;
    for (int p = 1; p < 3; ++p) if (total[p] < total[fastest]) fastest = p;
    printf("Fastest measured policy: %s\n", names[fastest]);
    puts("Verification: all policies match the serial reference at EVERY iteration.");
    free(expected); free(output);
    return 0;
}
