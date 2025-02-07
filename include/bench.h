#ifndef BENCH_H
#define BENCH_H

#include <string>
#include <vector>

#define CHUNK_SIZE 4096

typedef std::vector<int> Chunk;

void generate_file(const std::string &filename, size_t file_size_mb, int seed);
void sort_chunk(std::vector<int> &chunk);
void merge_sorted_chunks(const std::vector<std::string> &chunk_files, const std::string &output_file);
void ema_sort(const std::string &input_file, const std::string &output_file, int repeat_count);
void EmaSortInt(std::vector<int> &numbers, int repeat_count);

#endif // BENCH_H
