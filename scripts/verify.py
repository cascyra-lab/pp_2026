import numpy as np
import sys

def verify_results(input_file, output_file):
    with open(input_file, 'r') as f:
        N = int(f.readline().strip())
        A = np.zeros((N, N))
        B = np.zeros((N, N))
        
        for i in range(N):
            A[i] = list(map(float, f.readline().split()))
        for i in range(N):
            B[i] = list(map(float, f.readline().split()))

    C_ref = np.dot(A, B)
    C_cpp = np.loadtxt(output_file).reshape((N, N))
    
    diff = np.abs(C_ref - C_cpp)
    max_diff = np.max(diff)
    mean_diff = np.mean(diff)
    rel_diff = max_diff / np.max(np.abs(C_ref))
    
    print(f"N = {N}")
    print(f"Max absolute difference: {max_diff}")
    print(f"Mean absolute difference: {mean_diff}")
    print(f"Max relative difference: {rel_diff}")
    print(f"C++ result range: [{np.min(C_cpp)}, {np.max(C_cpp)}]")
    print(f"Python result range: [{np.min(C_ref)}, {np.max(C_ref)}]")
    
    is_correct = np.allclose(C_ref, C_cpp, rtol=1e-4, atol=1e-4)

    if is_correct:
        print(f"[OK] Verification passed")
        return 0
    else:
        print(f"[FAIL] Results mismatch!")
        return 1

if __name__ == "__main__":
    sys.exit(verify_results(sys.argv[1], sys.argv[2]))