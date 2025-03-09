//
// Created by andreas on 09.03.25.
//

#include "gtest/gtest.h"
#include "./../read_write_inputs.h"


TEST(TestGenerateWriteRead, simple1)
{
    std::vector<int> vector_sizes{1, 10, 100, 1000};
    auto generated_vectors = generate_random_vectors(vector_sizes);
    const std::string filename = "random_vectors.bin";
    write_vectors_to_file(filename, generated_vectors);
    auto read_vectors = read_vectors_from_file(filename);
    EXPECT_EQ(generated_vectors.size(), read_vectors.size());
    EXPECT_EQ(generated_vectors, read_vectors);
}
