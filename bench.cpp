#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>
#include "include/сache.h"

#define CHUNK_SIZE 4096  

void generate_file(const std::string &filename, size_t file_size_mb, int seed) {
    size_t total_bytes = file_size_mb * 1024 * 1024;
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Ошибка: не удалось создать файл!" << std::endl;
        return;
    }

    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 100);

    for (size_t i = 0; i < total_bytes / sizeof(int); ++i) {
        int num = dist(rng);
        file.write(reinterpret_cast<char*>(&num), sizeof(int));
    }

    file.close();
    std::cout << "✅ Файл \"" << filename << "\" успешно создан.\n";
}

void EmaSortInt(std::vector<int> &numbers, int repeat_count) {
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < repeat_count; ++i) {
        std::sort(numbers.begin(), numbers.end());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    std::cout << "⏳ Время сортировки: " << elapsed.count() << " секунд\n";
}

void ema_sort(const std::string &input_file, const std::string &output_file, int repeat_count, MRUCache &cache) {
    std::ifstream file(input_file, std::ios::binary);
    if (!file) {
        std::cerr << "Ошибка: не удалось открыть входной файл!" << std::endl;
        return;
    }

    std::vector<int> data;
    while (!file.eof()) {
        std::vector<int> chunk(CHUNK_SIZE / sizeof(int));
        file.read(reinterpret_cast<char*>(chunk.data()), CHUNK_SIZE);
        chunk.resize(file.gcount() / sizeof(int));

        if (!chunk.empty()) {
            EmaSortInt(chunk, repeat_count);
            data.insert(data.end(), chunk.begin(), chunk.end());
            cache.accessPage(chunk.front());
        }
    }
    file.close();

    if (data.empty()) {
        std::cerr << "⚠️ Входной файл пуст, сортировка не выполнена!" << std::endl;
        return;
    }

    // **Сохранение отсортированных данных в output_file**
    std::ofstream output(output_file, std::ios::binary);
    if (!output) {
        std::cerr << "Ошибка: не удалось открыть выходной файл!" << std::endl;
        return;
    }

    output.write(reinterpret_cast<char*>(data.data()), data.size() * sizeof(int));
    output.close();
    std::cout << "✅ Файл \"" << output_file << "\" успешно записан.\n";
}
