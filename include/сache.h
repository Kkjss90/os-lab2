#ifndef CACHE_H
#define CACHE_H

#include <cstddef>
#include <vector>
#include <list>
#include <unordered_map>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

constexpr size_t BLOCK_SIZE = 4096;
constexpr size_t CACHE_COUNT = 16;
constexpr size_t MAX_OPEN_FILES = 256;

struct CachePage {
    off_t offset;
    std::vector<char> data;
    bool dirty;
};

class Cache {
public:
    Cache(size_t capacity, size_t block_size);
    CachePage* getPage(off_t offset);
    void putPage(off_t offset, const std::vector<char>& data, bool dirty);
    void evict();
    void setFileDescriptor(int fd);

private:
    size_t capacity_;
    size_t block_size_;
    int fd_;
    std::list<CachePage> cache_list_;
    std::unordered_map<off_t, std::list<CachePage>::iterator> page_map_;
};

struct FileDescriptor {
    int fd;
    off_t offset;
    Cache cache;

    FileDescriptor(int file_desc);
};

extern std::vector<FileDescriptor*> fd_table;

int lab2_open(const char* path);
int lab2_close(int fd);
off_t lab2_lseek(int fd, off_t offset, int whence);
int lab2_fsync(int fd);
ssize_t lab2_read(int fd, void* buf, size_t count);
ssize_t lab2_write(int fd, const void* buf, size_t count);

#endif // CACHE_H
