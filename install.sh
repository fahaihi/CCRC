#!/bin/bash
g++ src/preprocess_cluster_without_n.cpp -O3 -march=native -fopenmp -std=c++11 -o src/preprocess_cluster_without_n.out
g++ src/get_clustering_parameter_K.cpp -fopenmp -std=c++11 -O3 -o src/get_clustering_parameter_K.out
g++ -std=c++11 -fopenmp -O3 src/clustering.cpp -o src/clustering.out
g++ src/select_prime_number.cpp -O3 -std=c++11 -o src/select_prime_number.out
chmod +x ./src/compressor1.sh