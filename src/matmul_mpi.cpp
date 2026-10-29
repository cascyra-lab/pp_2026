#include <iostream>
#include <fstream>
#include <vector>
#include <mpi.h>
#include <cmath>
#include <string>
#include <algorithm>

void read_matrix(const std::string& filename, int& N, std::vector<double>& A, std::vector<double>& B) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    file >> N;
    A.resize(N * N);
    B.resize(N * N);
    for (int i = 0; i < N * N; ++i) file >> A[i];
    for (int i = 0; i < N * N; ++i) file >> B[i];
    file.close();
}

void write_matrix(const std::string& filename, int N, const std::vector<double>& C) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            file << C[i * N + j] << " ";
        }
        file << "\n";
    }
    file.close();
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 3) {
        if (rank == 0) {
            std::cerr << "Usage: mpiexec -np <num_procs> " << argv[0] << " <input_file> <output_file>" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];

    int N = 0;
    std::vector<double> A, B, C;

    if (rank == 0) {
        read_matrix(input_file, N, A, B);
        C.resize(N * N, 0.0);
    }

    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (N <= 0) {
        MPI_Finalize();
        return 0;
    }

    if (rank != 0) B.resize(N * N);
    MPI_Bcast(B.data(), N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    int base_rows = N / size;
    int remainder = N % size;

    std::vector<int> sendcounts(size); 
    std::vector<int> displs(size);     

    for (int i = 0; i < size; ++i) {
        sendcounts[i] = (i < remainder ? base_rows + 1 : base_rows) * N;
        displs[i] = (i == 0) ? 0 : displs[i-1] + sendcounts[i-1];
    }

    int my_rows = sendcounts[rank] / N;
    std::vector<double> my_A(my_rows * N);
    std::vector<double> my_C(my_rows * N, 0.0);

    MPI_Scatterv(A.data(), sendcounts.data(), displs.data(), MPI_DOUBLE,
                 my_A.data(), my_rows * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double start_time = MPI_Wtime();

    for (int i = 0; i < my_rows; ++i) {
        for (int k = 0; k < N; ++k) {
            double a_ik = my_A[i * N + k];
            for (int j = 0; j < N; ++j) {
                my_C[i * N + j] += a_ik * B[k * N + j];
            }
        }
    }

    double end_time = MPI_Wtime();
    double exec_time = end_time - start_time;

    double global_exec_time = 0.0;
    MPI_Reduce(&exec_time, &global_exec_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Gatherv(my_C.data(), my_rows * N, MPI_DOUBLE,
                C.data(), sendcounts.data(), displs.data(), MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    if (rank == 0) {
        double flops = 2.0 * N * N * N;
        double gflops = (flops / global_exec_time) / 1e9;

        std::cout << "=== Execution Report ===" << std::endl;
        std::cout << "Matrix size (N): " << N << std::endl;
        std::cout << "Number of MPI processes: " << size << std::endl;
        std::cout << "Execution time: " << global_exec_time << " seconds" << std::endl;
        std::cout << "Performance: " << gflops << " GFLOPS" << std::endl;

        write_matrix(output_file, N, C);

        std::vector<double> C_ref(N * N, 0.0);
        for (int i = 0; i < N; ++i) {
            for (int k = 0; k < N; ++k) {
                double a_ik = A[i * N + k];
                for (int j = 0; j < N; ++j) {
                    C_ref[i * N + j] += a_ik * B[k * N + j];
                }
            }
        }

        double max_diff = 0.0;
        for (int i = 0; i < N * N; ++i) {
            double diff = std::abs(C[i] - C_ref[i]);
            if (diff > max_diff) max_diff = diff;
        }

        std::cout << "Verification - Max absolute difference: " << max_diff << std::endl;
        if (max_diff < 1e-4) {
            std::cout << "[OK] Verification passed" << std::endl;
        } else {
            std::cout << "[FAIL] Verification failed!" << std::endl;
        }
    }

    MPI_Finalize();
    return 0;
}