//
// Created by andreas on 09.03.25.
//
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <string>

// Generates random vectors of integers.
std::vector<std::vector<int>> generateRandomVectors(const std::vector<int>& vector_sizes,
                                                    unsigned seed = std::random_device{}())
{
    std::vector<std::vector<int>> vectors(vector_sizes.size());
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist(0, 1000000);

    for (const auto size : vector_sizes)
    {
        for (auto& vec : vectors)
        {
            vec.resize(size);
            for (size_t i = 0; i < size; ++i)
            {
                vec[i] = dist(gen);
            }
        }
    }
    return vectors;
}

void write_vectors_to_file(const std::string& filename, const std::vector<std::vector<int>>& vectors)
{
    std::ofstream output_file_stream(filename, std::ios::binary);
    if (!output_file_stream)
    {
        std::cerr << "Error opening file for writing: " << filename << "\n";
        return;
    }
    // Write the number of vectors.
    size_t number_of_vectors = vectors.size();
    output_file_stream.write(reinterpret_cast<const char*>(&number_of_vectors), sizeof(number_of_vectors));

    // Write each vector: first its size, then its data.
    for (const auto& vector : vectors)
    {
        size_t current_vector_size = vector.size();
        output_file_stream.write(reinterpret_cast<const char*>(&current_vector_size), sizeof(current_vector_size));
        output_file_stream.write(reinterpret_cast<const char*>(vector.data()), current_vector_size * sizeof(int));
    }
}


std::vector<std::vector<int>> read_vectors_from_file(const std::string& filename) {
    std::vector<std::vector<int>> vectors;
    std::ifstream input_file_stream(filename, std::ios::binary);
    if (!input_file_stream) {
        std::cerr << "Error opening file for reading: " << filename << "\n";
        return vectors;
    }
    // Read the number of vectors.
    size_t numVectors;
    input_file_stream.read(reinterpret_cast<char*>(&numVectors), sizeof(numVectors));
    vectors.resize(numVectors);

    // Read each vector.
    for (size_t i = 0; i < numVectors; ++i) {
        size_t current_vector_size;
        input_file_stream.read(reinterpret_cast<char*>(&current_vector_size), sizeof(current_vector_size));
        vectors[i].resize(current_vector_size);
        input_file_stream.read(reinterpret_cast<char*>(vectors[i].data()), current_vector_size * sizeof(int));
    }
    return vectors;
}

int main()
{
    // Configuration for random vectors.
    const std::string filename = "random_vectors.bin";
    std::vector<int> vector_sizes{10, 100, 1000, 10000, 100000, 1000000};
    // 1. Generate random vectors.
    auto vectors = generateRandomVectors(vector_sint main()
{
    // Configuration for random vectors.
    const std::string filename = "random_vectors.bin";
    std::vector<int> vector_sizes{10, 100, 1000, 10000, 100000, 1000000};
    // 1. Generate random vectors.
    auto vectors = generateRandomVectors(vector_sizes);

    // 2. Save them to disk.
    write_vectors_to_file(filename, vectors);
    auto read_vectors = read_vectors_from_file(filename);
    for(int i{}; i < vectors.size(); ++i)
    {
        for(int j{}; j < vectors[i].size(); ++j)
        {

        }
    }
    std::cout << "Saved vectors to file: " << filename << "\n";
    return 0;
}
izes);

    // 2. Save them to disk.
    write_vectors_to_file(filename, vectors);
    auto read_vectors = read_vectors_from_file(filename);
    for(int i{}; i < vectors.size(); ++i)
    {
        for(int j{}; j < vectors[i].size(); ++j)
        {

        }
    }
    std::cout << "Saved vectors to file: " << filename << "\n";
    return 0;
}
