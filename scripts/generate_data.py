import numpy as np
import sys

def generate_input(filename, N):
    A = np.random.uniform(-10, 10, (N, N))
    B = np.random.uniform(-10, 10, (N, N))
    
    with open(filename, 'w') as f:
        f.write(f"{N}\n")
        for row in A:
            f.write(" ".join(map(str, row)) + "\n")
        for row in B:
            f.write(" ".join(map(str, row)) + "\n")
    print(f"Generated {filename} for N={N}")

if __name__ == "__main__":
    N = int(sys.argv[1]) if len(sys.argv) > 1 else 500
    generate_input(f"data/input_{N}.txt", N)