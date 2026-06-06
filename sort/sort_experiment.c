#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <direct.h>
#pragma warning(disable: 4996)   /* 屏蔽VS的C4996安全函数警告 */

/* ========== 输出目录（纯英文路径，避免中文乱码/路径问题）========== */
#define OUT_DIR "D:\\AlgorithmExperiment\\sort\\output"

/* ========== 全局比较计数器 ========== */
long long cnt_bubble = 0;
long long cnt_merge  = 0;
long long cnt_quick  = 0;

/* ========== 子问题规模日志（任务③用）========== */
#define MAX_LOG 4000000
int merge_sub_log[MAX_LOG];
int merge_sub_cnt = 0;
int quick_sub_log[MAX_LOG];
int quick_sub_cnt = 0;

/* ========== 辅助函数 ========== */
void copyArr(int* dst, int* src, int n)
{
    memcpy(dst, src, n * sizeof(int));
}

/* ========== 优化冒泡排序 ========== */
void bubbleSort(int arr[], int n)
{
    cnt_bubble = 0;
    int i, j;
    for (i = 0; i < n - 1; i++)
    {
        int flag = 1;
        for (j = 0; j < n - i - 1; j++)
        {
            cnt_bubble++;
            if (arr[j] > arr[j + 1])
            {
                int t = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = t;
                flag = 0;
            }
        }
        if (flag == 1) break;
    }
}

/* ========== 归并排序 ========== */
void merge(int arr[], int l, int mid, int r)
{
    int n1 = mid - l + 1, n2 = r - mid;
    int* L = (int*)malloc(n1 * sizeof(int));
    int* R = (int*)malloc(n2 * sizeof(int));
    int i, j, k;
    for (i = 0; i < n1; i++) L[i] = arr[l + i];
    for (i = 0; i < n2; i++) R[i] = arr[mid + 1 + i];
    i = 0; j = 0; k = l;
    while (i < n1 && j < n2)
    {
        cnt_merge++;
        if (L[i] <= R[j]) arr[k++] = L[i++];
        else arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
    free(L); free(R);
}

/* 不记录子问题规模（任务①②用）*/
void mergeCore(int arr[], int l, int r)
{
    if (l < r)
    {
        int mid = l + (r - l) / 2;
        mergeCore(arr, l, mid);
        mergeCore(arr, mid + 1, r);
        merge(arr, l, mid, r);
    }
}

/* 记录子问题规模（任务③用）*/
void mergeCoreLog(int arr[], int l, int r)
{
    if (l < r)
    {
        int n = r - l + 1;
        if (merge_sub_cnt < MAX_LOG)
            merge_sub_log[merge_sub_cnt++] = n;
        int mid = l + (r - l) / 2;
        mergeCoreLog(arr, l, mid);
        mergeCoreLog(arr, mid + 1, r);
        merge(arr, l, mid, r);
    }
}

void mergeSort(int arr[], int n)
{
    cnt_merge = 0;
    mergeCore(arr, 0, n - 1);
}

void mergeSortLog(int arr[], int n)
{
    cnt_merge = 0;
    merge_sub_cnt = 0;
    mergeCoreLog(arr, 0, n - 1);
}

/* ========== 快速排序 ========== */
void quickCore(int arr[], int left, int right)
{
    if (left >= right) return;
    int pivot = arr[left];
    int i = left, j = right;
    while (i < j)
    {
        while (i < j)
        {
            cnt_quick++;
            if (arr[j] < pivot) break;
            j--;
        }
        arr[i] = arr[j];
        while (i < j)
        {
            cnt_quick++;
            if (arr[i] > pivot) break;
            i++;
        }
        arr[j] = arr[i];
    }
    arr[i] = pivot;
    quickCore(arr, left, i - 1);
    quickCore(arr, i + 1, right);
}

/* 记录子问题规模（任务③用）*/
void quickCoreLog(int arr[], int left, int right)
{
    if (left >= right) return;
    int n = right - left + 1;
    if (quick_sub_cnt < MAX_LOG)
        quick_sub_log[quick_sub_cnt++] = n;
    int pivot = arr[left];
    int i = left, j = right;
    while (i < j)
    {
        while (i < j)
        {
            cnt_quick++;
            if (arr[j] < pivot) break;
            j--;
        }
        arr[i] = arr[j];
        while (i < j)
        {
            cnt_quick++;
            if (arr[i] > pivot) break;
            i++;
        }
        arr[j] = arr[i];
    }
    arr[i] = pivot;
    quickCoreLog(arr, left, i - 1);
    quickCoreLog(arr, i + 1, right);
}

void quickSort(int arr[], int n)
{
    cnt_quick = 0;
    quickCore(arr, 0, n - 1);
}

void quickSortLog(int arr[], int n)
{
    cnt_quick = 0;
    quick_sub_cnt = 0;
    quickCoreLog(arr, 0, n - 1);
}

/* ==================================================================
 *  任务①：两组各100个随机数，对比三种算法比较次数，理解等价类
 *  要求：记录两组测试数据本身 + 比较操作执行次数，对比分析
 * ================================================================== */
void task1()
{
    int arr_src[100], arr[100];
    int seeds[2] = {42, 777};
    int g, i;
    char path[256];
    FILE* fp;
    FILE* fp_data;

    printf("\n============================================================\n");
    printf("  Task1: 2 groups of 100 random numbers (equivalence class)\n");
    printf("============================================================\n\n");

    /* ---- 输出1：两组完整随机数数据 ---- */
    sprintf(path, "%s\\task1_random_data.csv", OUT_DIR);
    fp_data = fopen(path, "w");
    if (!fp_data) { _mkdir(OUT_DIR); fp_data = fopen(path, "w"); }
    if (fp_data)
    {
        int data1[100], data2[100];
        int k;
        fprintf(fp_data, "Index,Group1_seed42,Group2_seed777\n");
        srand((unsigned)seeds[0]);
        for (k = 0; k < 100; k++) data1[k] = rand() % 10000;
        srand((unsigned)seeds[1]);
        for (k = 0; k < 100; k++) data2[k] = rand() % 10000;
        for (k = 0; k < 100; k++)
            fprintf(fp_data, "%d,%d,%d\n", k+1, data1[k], data2[k]);
        fclose(fp_data);
        printf("[Saved] %s\n", path);
    }

    /* ---- 输出2：比较次数对比 ---- */
    sprintf(path, "%s\\task1_result.csv", OUT_DIR);
    fp = fopen(path, "w");
    if (!fp) { _mkdir(OUT_DIR); fp = fopen(path, "w"); }
    if (fp) fprintf(fp, "Group,Seed,Algorithm,ComparisonCount\n");

    for (g = 0; g < 2; g++)
    {
        srand((unsigned)seeds[g]);
        for (i = 0; i < 100; i++) arr_src[i] = rand() % 10000;

        printf("--- Group %d (seed=%d) ---\n", g + 1, seeds[g]);
        printf("  All 100 numbers:\n  ");
        for (i = 0; i < 100; i++)
        {
            printf("%d ", arr_src[i]);
            if ((i + 1) % 20 == 0) printf("\n  ");
        }
        printf("\n");

        copyArr(arr, arr_src, 100);
        bubbleSort(arr, 100);
        printf("  BubbleSort  comparisons: %lld\n", cnt_bubble);
        if (fp) fprintf(fp, "Group%d,%d,BubbleSort,%lld\n", g+1, seeds[g], cnt_bubble);

        copyArr(arr, arr_src, 100);
        mergeSort(arr, 100);
        printf("  MergeSort   comparisons: %lld\n", cnt_merge);
        if (fp) fprintf(fp, "Group%d,%d,MergeSort,%lld\n", g+1, seeds[g], cnt_merge);

        copyArr(arr, arr_src, 100);
        quickSort(arr, 100);
        printf("  QuickSort   comparisons: %lld\n", cnt_quick);
        if (fp) fprintf(fp, "Group%d,%d,QuickSort,%lld\n", g+1, seeds[g], cnt_quick);
    }

    printf("\n  [Analysis]\n");
    printf("  BubbleSort (with early stop): different counts for different data\n");
    printf("  MergeSort: slightly different counts (merge comparison depends on data)\n");
    printf("  QuickSort: significantly different counts (pivot selection varies)\n");

    if (fp) { fclose(fp); printf("\n[Saved] %s\n", path); }
}

/* ==================================================================
 *  任务②：不同规模数据的比较次数对比，输出CSV供画图
 * ================================================================== */
void task2()
{
    int size_arr[] = { 10, 100, 1000, 2000, 5000, 10000, 100000 };
    int len = sizeof(size_arr) / sizeof(size_arr[0]);
    int idx, n, i;
    char path[256];
    FILE* fp;

    printf("\n============================================================\n");
    printf("  Task2: Comparison counts at different scales\n");
    printf("============================================================\n");

    sprintf(path, "%s\\task2_comparison_counts.csv", OUT_DIR);
    fp = fopen(path, "w");
    if (!fp) { _mkdir(OUT_DIR); fp = fopen(path, "w"); }
    if (fp) fprintf(fp, "N,BubbleSort,MergeSort,QuickSort\n");

    printf("N\t\tBubbleSort\tMergeSort\tQuickSort\n");

    for (idx = 0; idx < len; idx++)
    {
        n = size_arr[idx];
        int* src  = (int*)malloc(n * sizeof(int));
        int* arr_b = (int*)malloc(n * sizeof(int));
        int* arr_m = (int*)malloc(n * sizeof(int));
        int* arr_q = (int*)malloc(n * sizeof(int));

        srand(2026);
        for (i = 0; i < n; i++) src[i] = rand() % 10000;
        copyArr(arr_b, src, n);
        copyArr(arr_m, src, n);
        copyArr(arr_q, src, n);

        /* 冒泡排序（n>10000太慢，跳过）*/
        long long b_cnt;
        if (n <= 10000)
        {
            bubbleSort(arr_b, n);
            b_cnt = cnt_bubble;
        }
        else
        {
            b_cnt = -1;
        }

        mergeSort(arr_m, n);
        quickSort(arr_q, n);

        if (b_cnt < 0)
            printf("%d\t\tskipped\t\t%lld\t\t%lld\n", n, cnt_merge, cnt_quick);
        else
            printf("%d\t\t%lld\t\t%lld\t\t%lld\n", n, cnt_bubble, cnt_merge, cnt_quick);

        if (fp)
        {
            if (b_cnt < 0)
                fprintf(fp, "%d,skipped,%lld,%lld\n", n, cnt_merge, cnt_quick);
            else
                fprintf(fp, "%d,%lld,%lld,%lld\n", n, cnt_bubble, cnt_merge, cnt_quick);
        }

        free(src); free(arr_b); free(arr_m); free(arr_q);
    }

    if (fp) { fclose(fp); printf("\n[Saved] %s\n", path); }
}

/* ==================================================================
 *  任务③：归并/快速排序记录每次递归子问题规模，输出CSV
 * ================================================================== */
void task3()
{
    int size_arr[] = { 10, 100, 1000, 2000, 5000, 10000, 100000 };
    int len = sizeof(size_arr) / sizeof(size_arr[0]);
    int idx, n, i, show;
    char path[256];
    FILE* fp;

    printf("\n============================================================\n");
    printf("  Task3: Recursive sub-problem scale recording\n");
    printf("============================================================\n");

    sprintf(path, "%s\\task3_sub_problem_scale.csv", OUT_DIR);
    fp = fopen(path, "w");
    if (!fp) { _mkdir(OUT_DIR); fp = fopen(path, "w"); }
    if (fp) fprintf(fp, "Algorithm,InputSize_N,RecursiveCallCount,SubProblemScale_First50\n");

    for (idx = 0; idx < len; idx++)
    {
        n = size_arr[idx];
        int* src   = (int*)malloc(n * sizeof(int));
        int* arr_m = (int*)malloc(n * sizeof(int));
        int* arr_q = (int*)malloc(n * sizeof(int));
        int j;

        srand(2026);
        for (j = 0; j < n; j++) src[j] = rand() % 10000;

        /* MergeSort - record sub-problem scale */
        copyArr(arr_m, src, n);
        mergeSortLog(arr_m, n);
        printf("\n[MergeSort] n=%d  recursive_calls=%d\n", n, merge_sub_cnt);
        printf("  sub-problem scale (first 20): ");
        show = merge_sub_cnt < 20 ? merge_sub_cnt : 20;
        for (i = 0; i < show; i++) printf("%d ", merge_sub_log[i]);
        printf("\n");

        if (fp)
        {
            fprintf(fp, "MergeSort,%d,%d,", n, merge_sub_cnt);
            show = merge_sub_cnt < 50 ? merge_sub_cnt : 50;
            for (i = 0; i < show; i++) fprintf(fp, "%d ", merge_sub_log[i]);
            fprintf(fp, "\n");
        }

        /* QuickSort - record sub-problem scale */
        copyArr(arr_q, src, n);
        quickSortLog(arr_q, n);
        printf("[QuickSort] n=%d  recursive_calls=%d\n", n, quick_sub_cnt);
        printf("  sub-problem scale (first 20): ");
        show = quick_sub_cnt < 20 ? quick_sub_cnt : 20;
        for (i = 0; i < show; i++) printf("%d ", quick_sub_log[i]);
        printf("\n");

        if (fp)
        {
            fprintf(fp, "QuickSort,%d,%d,", n, quick_sub_cnt);
            show = quick_sub_cnt < 50 ? quick_sub_cnt : 50;
            for (i = 0; i < show; i++) fprintf(fp, "%d ", quick_sub_log[i]);
            fprintf(fp, "\n");
        }

        free(src); free(arr_m); free(arr_q);
    }

    if (fp) { fclose(fp); printf("\n[Saved] %s\n", path); }
}

/* ==================================================================
 *  主函数
 * ================================================================== */
int main()
{
    printf("=============================================================\n");
    printf("  Sorting Algorithm Experiment\n");
    printf("  Algorithm Design and Analysis - YNU 2026 Spring\n");
    printf("=============================================================\n");

    _mkdir(OUT_DIR);

    /* 验证输出目录是否可用 */
    {
        char testpath[256];
        FILE* ftest;
        sprintf(testpath, "%s\\_test.txt", OUT_DIR);
        ftest = fopen(testpath, "w");
        if (ftest)
        {
            fprintf(ftest, "OK\n");
            fclose(ftest);
            remove(testpath);
            printf("[OK] Output directory ready: %s\n\n", OUT_DIR);
        }
        else
        {
            printf("[ERROR] Cannot write to: %s\n", OUT_DIR);
            printf("  Trying D:\\algo_out instead...\n");
            _mkdir("D:\\algo_out");
            /* 如果默认路径不行，尝试备用路径 */
            ftest = fopen("D:\\algo_out\\_test.txt", "w");
            if (ftest)
            {
                fclose(ftest);
                remove("D:\\algo_out\\_test.txt");
                printf("[OK] Fallback directory ready: D:\\algo_out\n\n");
            }
            else
            {
                printf("[ERROR] No writable directory found! Files will not be saved.\n\n");
            }
        }
    }

    task1();
    task2();
    task3();

    printf("\n=============================================================\n");
    printf("  All tasks completed!\n");
    printf("  Output directory: %s\\\n", OUT_DIR);
    printf("    task1_random_data.csv       -- Task1: raw data\n");
    printf("    task1_result.csv            -- Task1: comparison counts\n");
    printf("    task2_comparison_counts.csv -- Task2: multi-scale data\n");
    printf("    task3_sub_problem_scale.csv -- Task3: sub-problem scale\n");
    printf("=============================================================\n");

    system("pause");
    return 0;
}
