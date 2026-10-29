import csv

with open('results/mpi_results.csv', 'r') as f:
    reader = csv.DictReader(f)
    rows = list(reader)

# Очистка пробелов в заголовках
clean_rows = []
for row in rows:
    clean_row = {k.strip(): v for k, v in row.items()}
    clean_rows.append(clean_row)

for row in clean_rows:
    n = int(row['N'])
    time_sec = float(row['Time_sec'])
    if time_sec > 0:
        flops = 2.0 * n * n * n
        gflops = (flops / time_sec) / 1e9
        row['GFLOPS'] = str(round(gflops, 4))

with open('results/mpi_results.csv', 'w', newline='') as f:
    writer = csv.DictWriter(f, fieldnames=['N', 'Processes', 'Time_sec', 'GFLOPS', 'Verification'])
    writer.writeheader()
    writer.writerows(clean_rows)

print("GFLOPS recalculated and saved!")
print(f"{'N':>6} {'Processes':>9} {'Time_sec':>12} {'GFLOPS':>10} {'Verification':>12}")
for row in clean_rows:
    print(f"{row['N']:>6} {row['Processes']:>9} {row['Time_sec']:>12} {row['GFLOPS']:>10} {row['Verification']:>12}")