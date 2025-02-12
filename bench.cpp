#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>
#include "include/bench.h"

#define CHUNK_SIZE 4096  // Размер чанка в байтах

//  Генератор случайного файла с числами
void generate_file(const std::string &filename, size_t file_size_mb, int seed) {
    size_t total_bytes = file_size_mb * 1024 * 1024;
    size_t total_ints = total_bytes / sizeof(int);
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Ошибка: не удалось создать файл!" << std::endl;
        return;
    }

    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 100);

    for (size_t i = 0; i < total_ints; ++i) {
        int num = dist(rng);
        file.write(reinterpret_cast<char*>(&num), sizeof(int));
    }

    file.close();
    std::cout << "✅ Файл \"" << filename << "\" с " << total_ints << " числами успешно создан.\n";
}

// Алгоритм EmaSortInt (замена стандартной сортировки в чанках)
void EmaSortInt(std::vector<int> &numbers, int repeat_count) {
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < repeat_count; ++i) {
        std::sort(numbers.begin(), numbers.end());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    std::cout << "⏳ Время сортировки (" << repeat_count << " повторов): " 
              << elapsed.count() << " секунд\n";
}

// Функция слияния отсортированных чанков в один файл
void merge_sorted_chunks(const std::vector<std::string> &chunk_files, const std::string &output_file) {
    std::ofstream output(output_file, std::ios::binary);
    if (!output) {
        std::cerr << "Ошибка: не удалось открыть выходной файл!" << std::endl;
        return;
    }

    std::vector<std::ifstream> inputs(chunk_files.size());
    auto cmp = [](const std::pair<int, size_t> &a, const std::pair<int, size_t> &b) {
        return a.first > b.first;
    };
    std::priority_queue<std::pair<int, size_t>, std::vector<std::pair<int, size_t>>, decltype(cmp)> min_heap(cmp);

    for (size_t i = 0; i < chunk_files.size(); ++i) {
        inputs[i].open(chunk_files[i], std::ios::binary);
        if (!inputs[i]) {
            std::cerr << "Ошибка открытия временного файла: " << chunk_files[i] << std::endl;
            continue;
        }
        int value;
        if (inputs[i].read(reinterpret_cast<char*>(&value), sizeof(int))) {
            min_heap.emplace(value, i);
        }
    }

    while (!min_heap.empty()) {
        auto [value, index] = min_heap.top();
        min_heap.pop();
        output.write(reinterpret_cast<char*>(&value), sizeof(int));

        int next_value;
        if (inputs[index].read(reinterpret_cast<char*>(&next_value), sizeof(int))) {
            min_heap.emplace(next_value, index);
        }
    }

    for (size_t i = 0; i < inputs.size(); ++i) {
        inputs[i].close();
        std::remove(chunk_files[i].c_str());
    }
    output.close();
    std::cout << "✅ Слияние завершено: файл \"" << output_file << "\" отсортирован.\n";
}

// Основная функция EMA-SORT
void ema_sort(const std::string &input_file, const std::string &output_file, int repeat_count) {
    std::ifstream file(input_file, std::ios::binary);
    if (!file) {
        std::cerr << "Ошибка: не удалось открыть входной файл!" << std::endl;
        return;
    }

    std::vector<std::string> chunk_files;
    size_t chunk_count = 0;

    while (!file.eof()) {
        std::vector<int> chunk(CHUNK_SIZE / sizeof(int));
        file.read(reinterpret_cast<char*>(chunk.data()), CHUNK_SIZE);
        size_t read_count = file.gcount() / sizeof(int);
        chunk.resize(read_count);

        if (!chunk.empty()) {
            EmaSortInt(chunk, repeat_count); 
            std::string chunk_file = "chunk_" + std::to_string(chunk_count++) + ".bin";
            chunk_files.push_back(chunk_file);
            std::ofstream chunk_out(chunk_file, std::ios::binary);
            chunk_out.write(reinterpret_cast<char*>(chunk.data()), chunk.size() * sizeof(int));
            chunk_out.close();
        }
    }
    file.close();

    merge_sorted_chunks(chunk_files, output_file);
}


// int main(int argc, char *argv[]) {
//     if (argc != 3) {
//         std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << std::endl;
//         return EXIT_FAILURE;
//     }

//     ema_sort(argv[1], argv[2]);
//     return EXIT_SUCCESS;
// }
