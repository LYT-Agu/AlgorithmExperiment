/*
 * ============================================================
 * 0-1 Knapsack Problem Experiment
 * Algorithm Design and Analysis - YNU 2026 Spring
 *
 * Algorithms: Brute Force (Meet-in-the-Middle), Dynamic Programming,
 *             Greedy, Backtracking (with bound pruning)
 *
 * Usage (Visual Studio):
 *   1. Create "Empty Project", add this file to "Source Files"
 *   2. Project Properties -> C/C++ -> Advanced -> Compile As -> "Compile as C Code (/TC)"
 *   3. Linker -> System -> Stack Reserve Size -> 16777216 (16MB)
 *   4. Ctrl+F5 to run
 *
 * Notes:
 *   - Brute Force: Meet-in-the-Middle, n<=40
 *   - DP: 1D rolling array, space O(C); small-scale solution reconstruction
 *   - Backtracking: with upper bound pruning, n<=20000
 *   - Time: clock(), unit ms, excludes data generation
 * ============================================================
 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <direct.h>

#define OUT_DIR "D:\\AlgorithmExperiment\\knapsack\\output"
#define MAX_DETAIL_PRINT 20
#define MAX_KEEP_CELLS   500000000LL  /* 500MB max for keep array */

/* ==================== Data Structure ==================== */
typedef struct {
    int    id;
    int    weight;
    double value;
    double ratio;
} Item;

typedef struct {
    long long w;
    double    v;
    int       mask;
} Subset;

/* ==================== Global Data ==================== */
static Item   *g_items    = NULL;
static int    *g_selected = NULL;
static int     g_n        = 0;
static long long g_cap    = 0;

/* ==================== Data Generation ==================== */
static void gen_items(int n, unsigned int seed)
{
    int i;
    srand(seed);
    for (i = 0; i < n; i++) {
        g_items[i].id     = i + 1;
        g_items[i].weight = rand() % 100 + 1;
        g_items[i].value  = (rand() % 90001) / 100.0 + 100.0;
        g_items[i].ratio  = g_items[i].value / (double)g_items[i].weight;
    }
}

/* ==================== Helper: print selected items ==================== */
static void print_selected_detail(int *sel, int n, long long cap,
                                   const char *algo_name, FILE *fp)
{
    long long tw = 0;
    int cnt = 0;
    int i;
    int printed = 0;

    if (!sel) return;
    for (i = 0; i < n; i++) if (sel[i]) cnt++;
    if (cnt == 0) return;

    /* console: limited output */
    printf("    >> %s selected items (show first %d of %d):\n",
           algo_name, (cnt < MAX_DETAIL_PRINT ? cnt : MAX_DETAIL_PRINT), cnt);
    printf("       %-6s %-8s %-10s\n", "ID", "Weight", "Value");
    for (i = 0; i < n && printed < MAX_DETAIL_PRINT; i++) {
        if (sel[i]) {
            printf("       %-6d %-8d %-10.2f\n",
                   g_items[i].id, g_items[i].weight, g_items[i].value);
            tw += g_items[i].weight;
            printed++;
        }
    }
    if (cnt > MAX_DETAIL_PRINT)
        printf("       ... (%d more items omitted)\n", cnt - MAX_DETAIL_PRINT);
    printf("       Total weight: %I64d / %I64d\n", tw, cap);

    /* file: full list */
    if (fp) {
        fprintf(fp, "\n--- %s (n=%d, C=%I64d) ---\n", algo_name, n, cap);
        fprintf(fp, "ItemID,Weight,Value\n");
        for (i = 0; i < n; i++) {
            if (sel[i])
                fprintf(fp, "%d,%d,%.2f\n",
                        g_items[i].id, g_items[i].weight, g_items[i].value);
        }
    }
}

/* ==================== Brute Force: Meet-in-the-Middle ==================== */
static int cmp_subset_w(const void *a, const void *b)
{
    long long d = ((const Subset *)a)->w - ((const Subset *)b)->w;
    return (d > 0) ? 1 : (d < 0 ? -1 : 0);
}

static double brute_force_mitm(int n, long long cap, int *result)
{
    int n1, n2, i, j, k;
    int total1, total2;
    Subset *left = NULL, *right = NULL;
    double *right_best_v = NULL;
    int    *right_best_m = NULL;
    double best_val = 0.0;
    int best_l = -1, best_r = -1;

    if (n > 40) return -1.0;

    n1 = n / 2;
    n2 = n - n1;
    total1 = 1 << n1;
    total2 = 1 << n2;

    left  = (Subset *)malloc((size_t)total1 * sizeof(Subset));
    right = (Subset *)malloc((size_t)total2 * sizeof(Subset));
    if (!left || !right) { free(left); free(right); return -2.0; }

    for (i = 0; i < total1; i++) {
        long long w = 0;
        double v = 0.0;
        for (j = 0; j < n1; j++)
            if (i & (1 << j)) { w += g_items[j].weight; v += g_items[j].value; }
        left[i].w = w; left[i].v = v; left[i].mask = i;
    }
    for (i = 0; i < total2; i++) {
        long long w = 0;
        double v = 0.0;
        for (j = 0; j < n2; j++)
            if (i & (1 << j)) { w += g_items[n1 + j].weight; v += g_items[n1 + j].value; }
        right[i].w = w; right[i].v = v; right[i].mask = i;
    }

    qsort(right, (size_t)total2, sizeof(Subset), cmp_subset_w);

    right_best_v = (double *)malloc((size_t)total2 * sizeof(double));
    right_best_m = (int *)malloc((size_t)total2 * sizeof(int));
    if (!right_best_v || !right_best_m) {
        free(left); free(right); free(right_best_v); free(right_best_m);
        return -2.0;
    }
    /* prefix max: for each position, track the BEST (value, mask) among right[0..i] */
    /* Array is sorted by weight ASCENDING, so binary search gives last idx with w<=rem. */
    /* We need the max value among elements 0..pos (all valid), NOT pos..end. */
    right_best_v[0] = right[0].v;
    right_best_m[0] = right[0].mask;
    for (i = 1; i < total2; i++) {
        if (right[i].v >= right_best_v[i - 1]) {
            right_best_v[i] = right[i].v;
            right_best_m[i] = right[i].mask;
        } else {
            right_best_v[i] = right_best_v[i - 1];
            right_best_m[i] = right_best_m[i - 1];
        }
    }

    for (i = 0; i < total1; i++) {
        long long rem = cap - left[i].w;
        if (rem < 0) continue;
        /* binary search largest idx with right[idx].w <= rem */
        int lo = 0, hi = total2 - 1, pos = -1;
        while (lo <= hi) {
            int mid = (lo + hi) / 2;
            if (right[mid].w <= rem) { pos = mid; lo = mid + 1; }
            else hi = mid - 1;
        }
        if (pos >= 0) {
            double total_v = left[i].v + right_best_v[pos];
            if (total_v > best_val) {
                best_val = total_v;
                best_l = left[i].mask;
                best_r = right_best_m[pos];
            }
        }
    }

    if (result && best_l >= 0) {
        memset(result, 0, (size_t)n * sizeof(int));
        for (j = 0; j < n1; j++)
            if (best_l & (1 << j)) result[j] = 1;
        for (j = 0; j < n2; j++)
            if (best_r & (1 << j)) result[n1 + j] = 1;
    }

    free(left); free(right); free(right_best_v); free(right_best_m);
    return best_val;
}

/* ==================== Dynamic Programming (1D rolling + keep array for small scale) ==================== */
static double dp_knapsack(int n, long long cap, int *result)
{
    double *dp = NULL;
    char   *keep = NULL;
    double best;
    int can_keep;
    long long cells;
    int i;
    long long c;
    long long total_w = 0;
    double total_v = 0.0;

    /* Fast path: if all items fit, select everything */
    for (i = 0; i < n; i++) {
        total_w += g_items[i].weight;
        total_v += g_items[i].value;
    }
    if (total_w <= cap) {
        if (result) {
            for (i = 0; i < n; i++) result[i] = 1;
        }
        return total_v;
    }

    if (cap > 2000000LL) return -2.0;

    dp = (double *)calloc((size_t)(cap + 1), sizeof(double));
    if (!dp) return -2.0;

    cells = (long long)n * (cap + 1);
    can_keep = (cells <= MAX_KEEP_CELLS && result != NULL);
    if (can_keep) {
        keep = (char *)calloc((size_t)cells, sizeof(char));
        if (!keep) can_keep = 0;
    }
    if (result) memset(result, 0, (size_t)n * sizeof(int));

    for (i = 0; i < n; i++) {
        for (c = cap; c >= (long long)g_items[i].weight; c--) {
            double cand = dp[c - g_items[i].weight] + g_items[i].value;
            if (cand > dp[c]) {
                dp[c] = cand;
                if (can_keep) keep[i * (cap + 1) + c] = 1;
            }
        }
    }

    best = dp[cap];

    /* reconstruct */
    if (can_keep && result) {
        long long rem = cap;
        for (i = n - 1; i >= 0 && rem > 0; i--) {
            if (keep[i * (cap + 1) + rem]) {
                result[i] = 1;
                rem -= g_items[i].weight;
            }
        }
    }

    free(dp);
    if (keep) free(keep);
    return best;
}

/* ==================== Greedy (by unit value desc) ==================== */
static int cmp_ratio_desc(const void *a, const void *b)
{
    double diff = ((const Item *)b)->ratio - ((const Item *)a)->ratio;
    return (diff > 1e-12) ? 1 : (diff < -1e-12 ? -1 : 0);
}

static double greedy_knapsack(int n, long long cap, int *result)
{
    Item *sorted;
    long long remain;
    double total;
    int i;

    sorted = (Item *)malloc((size_t)n * sizeof(Item));
    if (!sorted) return -2.0;
    memcpy(sorted, g_items, (size_t)n * sizeof(Item));
    qsort(sorted, (size_t)n, sizeof(Item), cmp_ratio_desc);

    if (result) memset(result, 0, (size_t)n * sizeof(int));

    remain = cap;
    total  = 0.0;
    for (i = 0; i < n; i++) {
        if ((long long)sorted[i].weight <= remain) {
            remain -= sorted[i].weight;
            total  += sorted[i].value;
            if (result) result[sorted[i].id - 1] = 1;
        }
    }
    free(sorted);
    return total;
}

/* ==================== Backtracking (no global state) ==================== */

typedef struct {
    double best_val;
    int   *result;
    int   *cur;
    int    n;
    Item  *items;
} BTState;

static double bt_upper_bound(const Item *items, int n, int idx, long long rem, double cur)
{
    double bound = cur;
    int i;
    for (i = idx; i < n; i++) {
        if ((long long)items[i].weight <= rem) {
            rem   -= items[i].weight;
            bound += items[i].value;
        } else {
            bound += items[i].value * ((double)rem / items[i].weight);
            break;
        }
    }
    return bound;
}

static void bt_recurse(BTState *st, int idx, long long rem, double cur)
{
    if (idx == st->n) {
        if (cur > st->best_val) {
            st->best_val = cur;
            memcpy(st->result, st->cur, (size_t)st->n * sizeof(int));
        }
        return;
    }
    if (bt_upper_bound(st->items, st->n, idx, rem, cur) <= st->best_val) return;

    if ((long long)st->items[idx].weight <= rem) {
        st->cur[idx] = 1;
        bt_recurse(st, idx + 1, rem - st->items[idx].weight,
                   cur + st->items[idx].value);
    }
    st->cur[idx] = 0;
    bt_recurse(st, idx + 1, rem, cur);
}

static double backtrack_knapsack(const Item *items_in, int n, long long cap, int *result)
{
    Item *sorted;
    int  *real_result;
    BTState st;
    double best;
    int i;

    if (n > 1000) return -1.0;  /* backtracking too slow for n>1000 */

    sorted = (Item *)malloc((size_t)n * sizeof(Item));
    if (!sorted) return -2.0;
    memcpy(sorted, items_in, (size_t)n * sizeof(Item));
    qsort(sorted, (size_t)n, sizeof(Item), cmp_ratio_desc);

    st.best_val = 0.0;
    st.n = n;
    st.items = sorted;
    st.result = (int *)calloc((size_t)n, sizeof(int));
    st.cur = (int *)calloc((size_t)n, sizeof(int));
    if (!st.result || !st.cur) {
        free(sorted); free(st.result); free(st.cur);
        return -2.0;
    }

    bt_recurse(&st, 0, cap, 0.0);

    if (result) {
        memset(result, 0, (size_t)n * sizeof(int));
        for (i = 0; i < n; i++)
            if (st.result[i]) result[sorted[i].id - 1] = 1;
    }

    best = st.best_val;
    free(st.cur);
    free(st.result);
    free(sorted);
    return best;
}

/* ==================== Print Summary ==================== */
static void print_summary(int *result, int n, long long cap,
                           double val, const char *name, double ms)
{
    long long total_w = 0;
    int cnt = 0, i;
    int has_result = 0;
    if (val < -1.5) {
        printf("  [%-12s] Out of memory / skipped\n", name);
        return;
    }
    if (val < 0.0) {
        printf("  [%-12s] Timeout skipped (n too large)\n", name);
        return;
    }
    if (result) {
        for (i = 0; i < n; i++) {
            if (result[i]) { total_w += g_items[i].weight; cnt++; has_result = 1; }
        }
    }
    if (has_result) {
        printf("  [%-12s] BestValue=%.2f  Selected=%d  Weight=%I64d/%I64d  Time=%.3fms\n",
               name, val, cnt, total_w, cap, ms);
    } else {
        printf("  [%-12s] BestValue=%.2f  Selected=N/A (reconstruction disabled)  Time=%.3fms\n",
               name, val, ms);
    }
}

/* ==================== Validation with small example ==================== */
static void validate_example(void)
{
    int n = 5;
    long long cap = 10;
    int sel[5];
    double v_bf, v_dp, v_gr, v_bt;
    Item *saved_items;
    int saved_n;
    Item orig[5] = {
        {1, 2, 6.0,  3.0},
        {2, 2, 3.0,  1.5},
        {3, 6, 5.0,  0.833},
        {4, 5, 4.0,  0.8},
        {5, 4, 6.0,  1.5}
    };
    int i;

    printf("\n========== Validation Example ==========\n");
    printf("  C=10, weights=[2,2,6,5,4], values=[6,3,5,4,6]\n");
    printf("  Expected: select items 1,2,5 -> total value=15\n\n");

    /* save global state */
    saved_items = g_items;
    saved_n     = g_n;

    /* temporarily point globals to our test data */
    g_items = orig;
    g_n     = n;

    /* --- Brute Force --- */
    memset(sel, 0, sizeof(sel));
    v_bf = brute_force_mitm(n, cap, sel);
    printf("  BruteForce  : BestValue=%.2f  Selected=", v_bf);
    for (i = 0; i < n; i++) if (sel[i]) printf("%d ", i + 1);
    printf("\n");

    /* --- DP --- */
    memset(sel, 0, sizeof(sel));
    v_dp = dp_knapsack(n, cap, sel);
    printf("  DP          : BestValue=%.2f  Selected=", v_dp);
    for (i = 0; i < n; i++) if (sel[i]) printf("%d ", i + 1);
    printf("\n");

    /* --- Greedy --- */
    memset(sel, 0, sizeof(sel));
    v_gr = greedy_knapsack(n, cap, sel);
    printf("  Greedy      : BestValue=%.2f  Selected=", v_gr);
    for (i = 0; i < n; i++) if (sel[i]) printf("%d ", i + 1);
    printf("\n");

    /* --- Backtracking --- */
    memset(sel, 0, sizeof(sel));
    v_bt = backtrack_knapsack(g_items, n, cap, sel);
    printf("  Backtracking: BestValue=%.2f  Selected=", v_bt);
    for (i = 0; i < n; i++) if (sel[i]) printf("%d ", i + 1);
    printf("\n");

    /* restore global state */
    g_items = saved_items;
    g_n     = saved_n;

    printf("\n  Validation %s\n",
           (v_bf == 15.0 && v_dp == 15.0 && v_bt == 15.0) ? "PASSED" : "FAILED");
    printf("========================================\n");
}

/* ==================== Run All Experiments ==================== */
static void run_experiment(void)
{
    int ns[] = {1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000,
                20000, 40000, 80000, 160000, 320000};
    int num_n = (int)(sizeof(ns) / sizeof(ns[0]));

    long long caps[] = {10000, 100000, 1000000};
    int num_c = (int)(sizeof(caps) / sizeof(caps[0]));

    FILE *fp_main   = NULL;
    FILE *fp_detail = NULL;
    char  path[256];
    int   ni, ci, i;
    int   n;
    long long cap;

    _mkdir(OUT_DIR);

    /* Main results CSV */
    sprintf(path, "%s\\knapsack_results.csv", OUT_DIR);
    fp_main = fopen(path, "w");
    if (!fp_main) { _mkdir(OUT_DIR); fp_main = fopen(path, "w"); }
    if (fp_main)
        fprintf(fp_main, "Algorithm,ItemCount_n,Capacity_C,BestValue,Time_ms,Space_bytes,Note\n");

    /* Detail output file */
    sprintf(path, "%s\\knapsack_selected_detail.txt", OUT_DIR);
    fp_detail = fopen(path, "w");
    if (!fp_detail) { _mkdir(OUT_DIR); fp_detail = fopen(path, "w"); }
    if (fp_detail)
        fprintf(fp_detail, "0-1 Knapsack Selected Items Detail\n");

    printf("\n===================================================\n");
    printf("  0-1 Knapsack Problem Experiment\n");
    printf("===================================================\n");

    for (ni = 0; ni < num_n; ni++) {
        n = ns[ni];

        g_items    = (Item *)malloc((size_t)n * sizeof(Item));
        g_selected = (int  *)calloc((size_t)n, sizeof(int));
        if (!g_items || !g_selected) {
            printf("Memory allocation failed (n=%d)\n", n);
            free(g_items); free(g_selected);
            g_items = NULL; g_selected = NULL;
            continue;
        }

        gen_items(n, 2026);

        printf("\n----- n = %d -----\n", n);

        for (ci = 0; ci < num_c; ci++) {
            cap = caps[ci];
            printf("\n  Capacity C = %I64d\n", cap);

            /* ---------- Brute Force ---------- */
            {
                double val = -1.0, ms = -1.0;
                const char *note = "";
                clock_t t0, t1;
                if (n <= 40) {
                    memset(g_selected, 0, (size_t)n * sizeof(int));
                    t0  = clock();
                    val = brute_force_mitm(n, cap, g_selected);
                    t1  = clock();
                    ms  = (double)(t1 - t0) * 1000.0 / CLOCKS_PER_SEC;
                    if (val >= 0.0)
                        print_selected_detail(g_selected, n, cap, "BruteForce", fp_detail);
                } else {
                    note = "n>40 skipped";
                }
                print_summary(g_selected, n, cap, val, "BruteForce", ms);
                if (fp_main)
                    fprintf(fp_main, "BruteForce,%d,%I64d,%.2f,%.3f,-,%s\n",
                            n, cap, val, ms, note);
            }

            /* ---------- DP ---------- */
            {
                double val, ms;
                long long mem;
                const char *note;
                clock_t t0, t1;
                long long nc = (long long)n * cap;
                int skip_dp = (nc > 10000000000LL);  /* 100亿次操作，约需几分钟 */
                memset(g_selected, 0, (size_t)n * sizeof(int));
                mem = (cap + 1) * (long long)sizeof(double);
                if (skip_dp) {
                    val = -1.0;
                    ms  = 0.0;
                    note = "n*C>10^10 skipped";
                    print_summary(NULL, n, cap, val, "DP", ms);
                } else {
                    t0  = clock();
                    val = dp_knapsack(n, cap, g_selected);
                    t1  = clock();
                    ms  = (double)(t1 - t0) * 1000.0 / CLOCKS_PER_SEC;
                    if (val < -1.5) {
                        note = "OOM skipped";
                    } else if (val >= 0.0 && g_selected && !g_selected[0]) {
                        note = "(reconstruction disabled for large n*C)";
                    } else {
                        note = "";
                    }
                    print_summary(g_selected, n, cap, val, "DP", ms);
                    if (val >= 0.0)
                        print_selected_detail(g_selected, n, cap, "DP", fp_detail);
                }
                if (fp_main)
                    fprintf(fp_main, "DP,%d,%I64d,%.2f,%.3f,%I64d,%s\n",
                            n, cap, val, ms, mem, note);
            }

            /* ---------- Greedy ---------- */
            {
                double val, ms;
                clock_t t0, t1;
                memset(g_selected, 0, (size_t)n * sizeof(int));
                t0  = clock();
                val = greedy_knapsack(n, cap, g_selected);
                t1  = clock();
                ms  = (double)(t1 - t0) * 1000.0 / CLOCKS_PER_SEC;
                print_summary(g_selected, n, cap, val, "Greedy", ms);
                if (val >= 0.0)
                    print_selected_detail(g_selected, n, cap, "Greedy", fp_detail);
                if (fp_main)
                    fprintf(fp_main, "Greedy,%d,%I64d,%.2f,%.3f,-,Not optimal\n",
                            n, cap, val, ms);
            }

            /* ---------- Backtracking ---------- */
            {
                double val = -1.0, ms = -1.0;
                const char *note = "";
                clock_t t0, t1;
                if (n <= 1000) {
                    memset(g_selected, 0, (size_t)n * sizeof(int));
                    t0  = clock();
                    val = backtrack_knapsack(g_items, n, cap, g_selected);
                    t1  = clock();
                    ms  = (double)(t1 - t0) * 1000.0 / CLOCKS_PER_SEC;
                    if (val >= 0.0)
                        print_selected_detail(g_selected, n, cap, "Backtracking", fp_detail);
                } else {
                    note = "n>1000 skipped";
                }
                print_summary(g_selected, n, cap, val, "Backtracking", ms);
                if (fp_main)
                    fprintf(fp_main, "Backtracking,%d,%I64d,%.2f,%.3f,-,%s\n",
                            n, cap, val, ms, note);
            }
        }

        free(g_items);    g_items    = NULL;
        free(g_selected); g_selected = NULL;
    }

    if (fp_main) {
        fclose(fp_main);
        printf("\n[Saved] %s\\knapsack_results.csv\n", OUT_DIR);
    }
    if (fp_detail) {
        fclose(fp_detail);
        printf("[Saved] %s\\knapsack_selected_detail.txt\n", OUT_DIR);
    }
}

/* ==================== Main ==================== */
int main(void)
{
    printf("=============================================\n");
    printf("  0-1 Knapsack Problem Experiment\n");
    printf("  Algorithm Design and Analysis - YNU 2026\n");
    printf("=============================================\n");

    validate_example();
    run_experiment();

    printf("\n============================================\n");
    printf("  All experiments completed!\n");
    printf("  Output directory: %s\\\n", OUT_DIR);
    printf("    knapsack_results.csv          -- main results\n");
    printf("    knapsack_selected_detail.txt  -- selected items detail\n");
    printf("============================================\n");

    return 0;
}
