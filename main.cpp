#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include "include/bench.h"

constexpr int FILE_SIZE = 256;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file> <repeat_count>\n";
        return EXIT_FAILURE;
    }

    const std::string input_file = argv[1];
    const std::string output_file = argv[2];
    const int repeat_count = std::stoi(argv[3]);

    int fd = open(input_file.c_str(), O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    close(fd);

    if (file_size == 0) {
        std::cout << "File not found or is empty. Generating file of size " << FILE_SIZE << " MB...\n";
        generate_file(input_file, FILE_SIZE, 1);
    } else {
        std::cout << "File already exists and is of size " << file_size << " bytes.\n";
    }

    clock_t start_time = clock();

    std::cout << "Sorting file using EMA-sort with MRU cache...\n";
    MRUCache cache(100);
    ema_sort(input_file, output_file, repeat_count, cache);

    clock_t end_time = clock();
    double duration = static_cast<double>(end_time - start_time) / CLOCKS_PER_SEC;
    std::cout << "Execution time: " << duration << " seconds\n";

    return EXIT_SUCCESS;
}
