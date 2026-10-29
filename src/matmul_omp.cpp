import std;
#include <omp.h>

void readMatrix(std::ifstream& in, std::vector<double>& mat, int N) {
    mat.resize(N * N);
    for (int i = 0; i < N * N; ++i) {
        in >> mat[i];
    }
}

void writeMatrix(std::ofstream& out, const std::vector<double>& mat, int N) {
    out << std::fixed << std::setprecision(6);
    for (int i = 0; i < N * N; ++i) {
        out << mat[i] << ((i + 1) % N == 0 ? "\n" : " ");
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input> <output> [threads]\n";
        return 1;
    }

    int num_threads = (argc > 3) ? std::atoi(argv[3]) : omp_get_max_threads();
    omp_set_num_threads(num_threads);

    std::ifstream in(argv[1]);
    if (!in.is_open()) {
        std::cerr << "Error opening input file!\n";
        return 1;
    }

    int N;
    in >> N;

    std::vector<double> A, B, C;
    readMatrix(in, A, N);
    readMatrix(in, B, N);
    in.close();

    C.assign(N * N, 0.0);

    auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel for schedule(static)
    for (int i = 0; i < N; ++i) {
        for (int k = 0; k < N; ++k) {
            double a_ik = A[i * N + k];
            for (int j = 0; j < N; ++j) {
                C[i * N + j] += a_ik * B[k * N + j];
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::ofstream out(argv[2]);
    writeMatrix(out, C, N);
    out.close();

    std::cout << "=== Report ===\n";
    std::cout << "N = " << N << " x " << N << "\n";
    std::cout << "Threads: " << num_threads << "\n";
    std::cout << "Time (s): " << duration.count() << "\n";
    std::cout << "GFLOPS: " << (2.0 * N * N * N) / (duration.count() * 1e9) << "\n";

    return 0;
}