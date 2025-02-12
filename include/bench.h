#ifndef BENCH_H
#define BENCH_H

#include <string>
#include "сache.h" // Подключаем MRUCache

void generate_file(const std::string &filename, size_t file_size_mb, int seed);
void EmaSortInt(std::vector<int> &numbers, int repeat_count);
void ema_sort(const std::string &input_file, const std::string &output_file, int repeat_count, MRUCache &cache);

#endif // BENCH_H
