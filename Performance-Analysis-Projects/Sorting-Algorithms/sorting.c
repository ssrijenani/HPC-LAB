#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Stop early if a complete Bubble Sort pass makes no swaps. */
void bubble_sort(int a[], int n) {
    for (int end = n - 1; end > 0; --end) {
        int swapped = 0;
        for (int j = 0; j < end; ++j) {
            if (a[j] > a[j + 1]) {
                int t = a[j]; a[j] = a[j + 1]; a[j + 1] = t;
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
}

void selection_sort(int a[], int n) {
    for (int i = 0; i < n - 1; ++i) {
        int smallest = i;
        for (int j = i + 1; j < n; ++j)
            if (a[j] < a[smallest]) smallest = j;
        int t = a[i]; a[i] = a[smallest]; a[smallest] = t;
    }
}

/* Merge two sorted halves using one reusable temporary array. */
void merge(int a[], int temp[], int left, int middle, int right) {
    int i = left, j = middle + 1, k = left;
    while (i <= middle && j <= right)
        temp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    while (i <= middle) temp[k++] = a[i++];
    while (j <= right) temp[k++] = a[j++];
    for (i = left; i <= right; ++i) a[i] = temp[i];
}

void merge_sort(int a[], int temp[], int left, int right) {
    if (left >= right) return;
    int middle = left + (right - left) / 2;
    merge_sort(a, temp, left, middle);
    merge_sort(a, temp, middle + 1, right);
    merge(a, temp, left, middle, right);
}

static int compare(const void *p, const void *q) {
    int a = *(const int *)p, b = *(const int *)q;
    return (a > b) - (a < b);
}

static void run_sort(int policy, int a[], int temp[], int n) {
    if (policy == 0) bubble_sort(a, n);
    else if (policy == 1) selection_sort(a, n);
    else merge_sort(a, temp, 0, n - 1);
}

static double seconds(void) {
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) { perror("clock_gettime"); exit(1); }
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

static int self_test(void) {
    int cases[][8] = {{0}, {7}, {1,2,3,4,5}, {5,4,3,2,1},
                     {3,3,1,3,1,0}, {-9,4,-1,0,4,-9}, {2,1}};
    int sizes[] = {0,1,5,5,6,6,2};
    for (int c = 0; c < 7; ++c) {
        int expected[8], a[8], temp[8];
        memcpy(expected, cases[c], sizeof expected);
        qsort(expected, (size_t)sizes[c], sizeof(int), compare);
        for (int p = 0; p < 3; ++p) {
            memcpy(a, cases[c], sizeof a);
            run_sort(p, a, temp, sizes[c]);
            if (memcmp(a, expected, (size_t)sizes[c] * sizeof(int)) != 0) return 0;
        }
    }
    puts("Self-tests: PASS (7 cases x 3 algorithms: empty, singleton, sorted, reverse, duplicates, negatives, pair)");
    return 1;
}

int main(int argc, char *argv[]) {
    if (!self_test()) { fputs("Self-test FAILED\n", stderr); return 1; }
    if (argc == 2 && strcmp(argv[1], "--test") == 0) return 0;
    /* A bounded size keeps the quadratic algorithms practical for beginners. */
    int n = 18000, trials = 3;
    if (argc > 2) { fputs("Usage: ./sorting [--test | size 1..50000]\n", stderr); return 1; }
    if (argc == 2) {
        char *end;
        long value = strtol(argv[1], &end, 10);
        if (*end || value < 1 || value > 50000) { fputs("Invalid size\n", stderr); return 1; }
        n = (int)value;
    }
    size_t bytes = (size_t)n * sizeof(int);
    int *input = malloc(bytes), *a = malloc(bytes), *temp = malloc(bytes), *expected = malloc(bytes);
    if (!input || !a || !temp || !expected) { fputs("Allocation failed\n", stderr); free(input); free(a); free(temp); free(expected); return 1; }
    uint32_t seed = 42;
    for (int i = 0; i < n; ++i) {
        seed = seed * UINT32_C(1664525) + UINT32_C(1013904223);
        input[i] = (int)(seed % 100000);
    }
    memcpy(expected, input, bytes);
    qsort(expected, (size_t)n, sizeof(int), compare);
    double total[3] = {0}, minimum[3] = {1e100,1e100,1e100};
    /* Rotate execution order; copy and correctness checks are outside timing. */
    for (int trial = 0; trial < trials; ++trial) {
        for (int offset = 0; offset < 3; ++offset) {
            int p = (trial + offset) % 3;
            memcpy(a, input, bytes);
            double start = seconds();
            run_sort(p, a, temp, n);
            double elapsed = seconds() - start;
            if (memcmp(a, expected, bytes) != 0) { fputs("Sort FAILED\n", stderr); free(input); free(a); free(temp); free(expected); return 1; }
            total[p] += elapsed;
            if (elapsed < minimum[p]) minimum[p] = elapsed;
        }
    }
    const char *names[] = {"Bubble Sort", "Selection Sort", "Merge Sort"};
    printf("Input: %d integers; seed=42; trials=%d; identical input for every run\n", n, trials);
    puts("Algorithm       | Mean seconds | Minimum seconds | Correct");
    for (int p = 0; p < 3; ++p)
        printf("%-15s | %.9f  | %.9f     | PASS\n", names[p], total[p] / trials, minimum[p]);
    int slowest = 0;
    for (int p = 1; p < 3; ++p) if (total[p] > total[slowest]) slowest = p;
    printf("Slowest measured algorithm: %s\n", names[slowest]);
    puts("Verification: every output element equals the independently qsort-sorted reference.");
    free(input); free(a); free(temp); free(expected);
    return 0;
}
