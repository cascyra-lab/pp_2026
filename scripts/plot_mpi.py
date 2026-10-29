import pandas as pd
import matplotlib.pyplot as plt

# 1. Читаем данные с суперкомпьютера
df = pd.read_csv('results/mpi_results.csv')

# 2. Рассчитываем ускорение (Speedup) = Время(1 процесс) / Время(p процессов)
speedup_data = []
for n in df['N'].unique():
    df_n = df[df['N'] == n].sort_values('Processes')
    t1 = df_n[df_n['Processes'] == 1]['Time_sec'].values[0]
    for _, row in df_n.iterrows():
        speedup = t1 / row['Time_sec']
        speedup_data.append({'N': n, 'Processes': row['Processes'], 'Speedup': speedup})

df_speedup = pd.DataFrame(speedup_data)

# 3. Настраиваем фигуру с двумя графиками
plt.style.use('seaborn-v0_8-darkgrid')
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))

# --- График 1: Время выполнения от размера матрицы ---
for p in sorted(df['Processes'].unique()):
    df_p = df[df['Processes'] == p].sort_values('N')
    ax1.plot(df_p['N'], df_p['Time_sec'], marker='o', linewidth=2, label=f'{p} проц.')

ax1.set_xlabel('Размер матрицы (N)')
ax1.set_ylabel('Время выполнения (сек)')
ax1.set_title('Зависимость времени от размера задачи (логарифмическая шкала)')
ax1.set_xscale('log', base=2)
ax1.set_yscale('log')
ax1.legend(title='Количество процессов')
ax1.grid(True, which="both", ls="--", alpha=0.7)

# --- График 2: Ускорение от числа процессов ---
# Покажем для двух характерных размеров: малого (400) и большого (2000)
target_Ns = [400, 2000]
colors = ['#1f77b4', '#d62728'] # синий и красный

for n, color in zip(target_Ns, colors):
    df_s = df_speedup[df_speedup['N'] == n].sort_values('Processes')
    ax2.plot(df_s['Processes'], df_s['Speedup'], marker='s', color=color, linewidth=2, label=f'N={n}')

# Линия идеального линейного ускорения
ideal_processes = [1, 2, 4, 8]
ax2.plot(ideal_processes, ideal_processes, 'k--', linewidth=2, label='Идеальное (S=P)')

ax2.set_xlabel('Количество процессов (P)')
ax2.set_ylabel('Ускорение (Speedup, S)')
ax2.set_title('Ускорение параллельной версии MPI')
ax2.set_xticks([1, 2, 4, 8])
ax2.set_ylim(0, 9)
ax2.legend()
ax2.grid(True, alpha=0.7)

# 4. Сохраняем результат
plt.tight_layout()
plt.savefig('results/mpi_plots.png', dpi=300, bbox_inches='tight')
print("✅ Графики успешно сохранены в results/mpi_plots.png")