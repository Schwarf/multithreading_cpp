//
// Created by andreas on 09.03.25.
//
#include "read_write_inputs.h"

int main()
{
    std::vector<int> vector_sizes{10, 100, 1000, 10000, 100000, 1000000, 10000000   };
    auto generated_vectors = generate_random_vectors(vector_sizes);
    const std::string filename{"random_vectors.bin"};
    write_vectors_to_file(filename, generated_vectors);
}
