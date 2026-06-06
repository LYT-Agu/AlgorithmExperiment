import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import csv
import os

# ========== 读取数据 ==========
csv_path = r"D:\AlgorithmExperiment\sort\output\task2_comparison_counts.csv"
out_dir  = r"D:\AlgorithmExperiment\sort\output"

ns       = []
bubble   = []
merge    = []
quick    = []

with open(csv_path, newline='', encoding='utf-8') as f:
    reader = csv.DictReader(f)
    for row in reader:
        n = int(row['N'])
        b = row['BubbleSort'].strip()
        m = row['MergeSort'].strip()
        q = row['QuickSort'].strip()
        # 跳过冒泡被标记为 skipped 的规模（用于全量图）
        ns.append(n)
        bubble.append(None if b.lower() == 'skipped' else int(b))
        merge.append(int(m))
        quick.append(int(q))

# ========== 图1：全量对比（含冒泡，N=100000冒泡跳过）==========
fig1, ax1 = plt.subplots(figsize=(10, 6))

# 只画有值的冒泡点
ns_b = [ns[i] for i in range(len(ns)) if bubble[i] is not None]
b_v  = [bubble[i] for i in range(len(ns)) if bubble[i] is not None]

ax1.plot(ns_b, b_v,  'o-', color='#e74c3c', linewidth=2, markersize=6, label='BubbleSort')
ax1.plot(ns,   merge, 's-', color='#2ecc71', linewidth=2, markersize=6, label='MergeSort')
ax1.plot(ns,   quick, '^-', color='#3498db', linewidth=2, markersize=6, label='QuickSort')

ax1.set_xlabel('Input Size (n)', fontsize=13)
ax1.set_ylabel('Comparison Count', fontsize=13)
ax1.set_title('Sorting Algorithms: Comparison Count vs Input Size', fontsize=14, fontweight='bold')
ax1.legend(fontsize=12)
ax1.grid(True, linestyle='--', alpha=0.5)
ax1.ticklabel_format(style='sci', axis='y', scilimits=(0,0))

plt.tight_layout()
path1 = os.path.join(out_dir, 'fig1_all_algorithms.png')
plt.savefig(path1, dpi=150)
plt.close()
print(f"[Saved] {path1}")

# ========== 图2：归并 vs 快排（O(n log n) 级别的对比，去掉冒泡更清晰）==========
fig2, ax2 = plt.subplots(figsize=(10, 6))

ax2.plot(ns, merge, 's-', color='#2ecc71', linewidth=2, markersize=6, label='MergeSort')
ax2.plot(ns, quick, '^-', color='#3498db', linewidth=2, markersize=6, label='QuickSort')

ax2.set_xlabel('Input Size (n)', fontsize=13)
ax2.set_ylabel('Comparison Count', fontsize=13)
ax2.set_title('MergeSort vs QuickSort: Comparison Count vs Input Size', fontsize=14, fontweight='bold')
ax2.legend(fontsize=12)
ax2.grid(True, linestyle='--', alpha=0.5)
ax2.ticklabel_format(style='sci', axis='y', scilimits=(0,0))

plt.tight_layout()
path2 = os.path.join(out_dir, 'fig2_merge_vs_quick.png')
plt.savefig(path2, dpi=150)
plt.close()
print(f"[Saved] {path2}")

# ========== 图3：对数坐标（更直观看增长率差异）==========
fig3, ax3 = plt.subplots(figsize=(10, 6))

ax3.loglog(ns_b, b_v,  'o-', color='#e74c3c', linewidth=2, markersize=6, label='BubbleSort  O(n²)')
ax3.loglog(ns,   merge, 's-', color='#2ecc71', linewidth=2, markersize=6, label='MergeSort  O(n log n)')
ax3.loglog(ns,   quick, '^-', color='#3498db', linewidth=2, markersize=6, label='QuickSort  O(n log n)')

ax3.set_xlabel('Input Size (n)  [log scale]', fontsize=13)
ax3.set_ylabel('Comparison Count  [log scale]', fontsize=13)
ax3.set_title('Log-Log Plot: Comparison Count vs Input Size', fontsize=14, fontweight='bold')
ax3.legend(fontsize=12)
ax3.grid(True, which='both', linestyle='--', alpha=0.5)

plt.tight_layout()
path3 = os.path.join(out_dir, 'fig3_loglog.png')
plt.savefig(path3, dpi=150)
plt.close()
print(f"[Saved] {path3}")

print("\nAll charts generated successfully!")
print(f"Output directory: {out_dir}")
