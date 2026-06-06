"""
0-1 Knapsack Experiment - Final Plotting Script
生成实验要求的折线图：
1. 各算法执行时间随物品数量变化（不同容量下）
2. 各算法空间占用对比
"""

import csv
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import os

# ── Matplotlib Chinese support ──────────────────────────────
matplotlib.rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei']
matplotlib.rcParams['axes.unicode_minus'] = False

# ── Paths ───────────────────────────────────────────────────
BASE  = r'D:\AlgorithmExperiment\knapsack'
OUT   = os.path.join(BASE, 'output')
CSV   = os.path.join(OUT, 'knapsack_results.csv')

# ── Read data ───────────────────────────────────────────────
rows = []
with open(CSV, 'r', encoding='utf-8') as f:
    reader = csv.DictReader(f)
    for r in reader:
        rows.append(r)

def to_float(val, default=-1):
    try:
        return float(val)
    except (ValueError, TypeError):
        return default

def to_int(val, default=-1):
    try:
        return int(val)
    except (ValueError, TypeError):
        return default

data = []
for r in rows:
    algo = r['Algorithm'].strip()
    n = to_int(r['ItemCount_n'])
    cap = to_int(r['Capacity_C'])
    best = to_float(r['BestValue'])
    time_ms = to_float(r['Time_ms'])
    space = to_int(r['Space_bytes'])
    note = r.get('Note', '').strip()
    if n < 0:
        continue
    data.append({
        'algo': algo, 'n': n, 'cap': cap,
        'best': best, 'time_ms': time_ms, 'time_s': time_ms / 1000.0,
        'space': space, 'note': note
    })

def filter_data(algo=None, cap=None):
    return [d for d in data
            if (algo is None or d['algo'] == algo)
            and (cap is None or d['cap'] == cap)]

# ── Color / marker / label maps ─────────────────────────────
colors = {
    'BruteForce': '#9C27B0',
    'DP': '#2196F3',
    'Greedy': '#4CAF50',
    'Backtracking': '#FF9800'
}
markers = {
    'BruteForce': 'D',
    'DP': 'o',
    'Greedy': 's',
    'Backtracking': '^'
}
labels = {
    'BruteForce': '蛮力法 (MITM)',
    'DP': '动态规划',
    'Greedy': '贪心算法',
    'Backtracking': '回溯法'
}

# ════════════════════════════════════════════════════════════
# Figure 1: 四种算法执行时间对比 (C=10,000)
# ════════════════════════════════════════════════════════════
fig1, ax1 = plt.subplots(figsize=(11, 7))

for algo in ['BruteForce', 'DP', 'Greedy', 'Backtracking']:
    sub = filter_data(algo=algo, cap=10000)
    if not sub:
        continue
    sub.sort(key=lambda d: d['n'])
    ns = [d['n'] for d in sub]
    ts = [max(d['time_s'], 0.0001) for d in sub]
    ax1.plot(ns, ts, marker=markers.get(algo, 'o'), linewidth=2, markersize=6,
             color=colors.get(algo, '#333'), label=labels.get(algo, algo))

ax1.set_xlabel('物品数量 n', fontsize=14)
ax1.set_ylabel('运行时间 (秒)', fontsize=14)
ax1.set_title('四种算法执行时间对比 (C=10,000)', fontsize=16, fontweight='bold')
ax1.legend(fontsize=12, loc='upper left')
ax1.grid(True, alpha=0.3)
ax1.set_xscale('log')
ax1.set_yscale('log')
ax1.tick_params(labelsize=12)

# 添加理论复杂度标注
ax1.annotate('O(n·C)', xy=(5000, 1), fontsize=11, color='#2196F3', alpha=0.7)
ax1.annotate('O(n log n)', xy=(50000, 0.001), fontsize=11, color='#4CAF50', alpha=0.7)
ax1.annotate('O(2^(n/2))', xy=(40, 0.01), fontsize=11, color='#9C27B0', alpha=0.7)

plt.tight_layout()
fig1.savefig(os.path.join(OUT, 'fig1_all_algorithms.png'), dpi=200, bbox_inches='tight')
print('[Saved] fig1_all_algorithms.png')

# ════════════════════════════════════════════════════════════
# Figure 2: 各算法在不同容量下的时间对比
# ════════════════════════════════════════════════════════════
fig2, axes = plt.subplots(1, 3, figsize=(18, 6))

capacities = [10000, 100000, 1000000]
for idx, cap in enumerate(capacities):
    ax = axes[idx]
    for algo in ['DP', 'Greedy', 'Backtracking']:
        sub = filter_data(algo=algo, cap=cap)
        if not sub:
            continue
        sub.sort(key=lambda d: d['n'])
        ns = [d['n'] for d in sub]
        ts = [max(d['time_s'], 0.0001) for d in sub]
        ax.plot(ns, ts, marker=markers.get(algo, 'o'), linewidth=2, markersize=5,
                color=colors.get(algo, '#333'), label=labels.get(algo, algo))
    
    ax.set_xlabel('物品数量 n', fontsize=12)
    ax.set_ylabel('运行时间 (秒)', fontsize=12)
    ax.set_title(f'C = {cap:,}', fontsize=14, fontweight='bold')
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.3)
    ax.set_xscale('log')
    ax.set_yscale('log')
    ax.tick_params(labelsize=10)

fig2.suptitle('三种算法在不同容量下的执行时间对比', fontsize=16, fontweight='bold', y=1.02)
plt.tight_layout()
fig2.savefig(os.path.join(OUT, 'fig2_capacity_compare.png'), dpi=200, bbox_inches='tight')
print('[Saved] fig2_capacity_compare.png')

# ════════════════════════════════════════════════════════════
# Figure 3: DP 空间占用随容量变化
# ════════════════════════════════════════════════════════════
fig3, ax3 = plt.subplots(figsize=(10, 6))

for n_val in [1000, 5000, 10000]:
    sub = [d for d in data if d['algo'] == 'DP' and d['n'] == n_val and d['space'] > 0]
    if not sub:
        continue
    sub.sort(key=lambda d: d['cap'])
    caps = [d['cap'] for d in sub]
    space_mb = [d['space'] / (1024 * 1024) for d in sub]
    ax3.plot(caps, space_mb, marker='o', linewidth=2, markersize=6,
             label=f'n={n_val:,}')

ax3.set_xlabel('背包容量 C', fontsize=14)
ax3.set_ylabel('空间占用 (MB)', fontsize=14)
ax3.set_title('动态规划空间占用随容量变化', fontsize=16, fontweight='bold')
ax3.legend(fontsize=12, title='物品数量 n', title_fontsize=12)
ax3.grid(True, alpha=0.3)
ax3.tick_params(labelsize=12)

plt.tight_layout()
fig3.savefig(os.path.join(OUT, 'fig3_dp_space.png'), dpi=200, bbox_inches='tight')
print('[Saved] fig3_dp_space.png')

# ════════════════════════════════════════════════════════════
# Figure 4: 贪心近似比
# ════════════════════════════════════════════════════════════
fig4, ax4 = plt.subplots(figsize=(10, 6))

for cap in [10000, 100000, 1000000]:
    dp_data = {d['n']: d['best'] for d in data if d['algo'] == 'DP' and d['cap'] == cap and d['best'] > 0}
    gr_data = {d['n']: d['best'] for d in data if d['algo'] == 'Greedy' and d['cap'] == cap and d['best'] > 0}
    common_n = sorted(set(dp_data.keys()) & set(gr_data.keys()))
    if not common_n:
        continue
    ratios = [gr_data[n] / dp_data[n] for n in common_n]
    ax4.plot(common_n, ratios, marker=markers.get('Greedy', 's'), linewidth=2, markersize=5,
             label=f'C={cap:,}')

ax4.axhline(y=1.0, color='gray', linestyle='--', alpha=0.5, label='最优比=1.0')
ax4.set_xlabel('物品数量 n', fontsize=14)
ax4.set_ylabel('贪心解 / 最优解', fontsize=14)
ax4.set_title('贪心算法近似比', fontsize=16, fontweight='bold')
ax4.legend(fontsize=12)
ax4.grid(True, alpha=0.3)
ax4.set_xscale('log')
ax4.tick_params(labelsize=12)

plt.tight_layout()
fig4.savefig(os.path.join(OUT, 'fig4_greedy_ratio.png'), dpi=200, bbox_inches='tight')
print('[Saved] fig4_greedy_ratio.png')

plt.close('all')
print('\nAll figures generated successfully!')
