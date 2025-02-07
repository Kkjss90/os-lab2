#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <list>
#include <unordered_map>

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
    Cache(size_t capacity, size_t block_size)
        : capacity_(capacity), block_size_(block_size) {}

    CachePage* getPage(off_t offset) {
        auto it = page_map_.find(offset);
        if (it != page_map_.end()) {
            cache_list_.splice(cache_list_.begin(), cache_list_, it->second);
            return &*(it->second);
        }
        return nullptr;
    }

    void putPage(off_t offset, const std::vector<char>& data, bool dirty) {
        if (cache_list_.size() >= capacity_) {
            evict();
        }
        cache_list_.emplace_front(CachePage{offset, data, dirty});
        page_map_[offset] = cache_list_.begin();
    }

    void evict() {
        if (!cache_list_.empty()) {
            auto& page = cache_list_.back();
            if (page.dirty) {
                pwrite(fd_, page.data.data(), block_size_, page.offset);
            }
            page_map_.erase(page.offset);
            cache_list_.pop_back();
        }
    }

    void setFileDescriptor(int fd) { fd_ = fd; }

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

    FileDescriptor(int file_desc)
        : fd(file_desc), offset(0), cache(CACHE_COUNT, BLOCK_SIZE) {
        cache.setFileDescriptor(fd);
    }
};

std::vector<FileDescriptor*> fd_table(MAX_OPEN_FILES, nullptr);

int lab2_open(const char* path) {
    int fd = open(path, O_RDWR | O_DIRECT);
    if (fd < 0) return -1;

    for (size_t i = 0; i < MAX_OPEN_FILES; ++i) {
        if (!fd_table[i]) {
            fd_table[i] = new FileDescriptor(fd);
            return i;
        }
    }

    close(fd);
    return -1;
}

int lab2_close(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fd_table[fd]) return -1;
    
    close(fd_table[fd]->fd);
    delete fd_table[fd];
    fd_table[fd] = nullptr;
    return 0;
}

off_t lab2_lseek(int fd, off_t offset, int whence) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fd_table[fd]) return -1;

    switch (whence) {
        case SEEK_SET:
            fd_table[fd]->offset = offset;
            break;
        case SEEK_CUR:
            fd_table[fd]->offset += offset;
            break;
        case SEEK_END: {
            struct stat st;
            if (fstat(fd_table[fd]->fd, &st) < 0) return -1;
            fd_table[fd]->offset = st.st_size + offset;
            break;
        }
        default:
            return -1;
    }
    return fd_table[fd]->offset;
}

int lab2_fsync(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fd_table[fd]) return -1;
    return fsync(fd_table[fd]->fd);
}

ssize_t lab2_read(int fd, void* buf, size_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fd_table[fd]) return -1;
    FileDescriptor* file = fd_table[fd];
    Cache& cache = file->cache;

    off_t block_offset = file->offset / BLOCK_SIZE * BLOCK_SIZE;
    size_t block_index = file->offset % BLOCK_SIZE;

    CachePage* page = cache.getPage(block_offset);
    if (!page) {
        std::vector<char> data(BLOCK_SIZE);
        ssize_t read_bytes = pread(file->fd, data.data(), BLOCK_SIZE, block_offset);
        if (read_bytes <= 0) return read_bytes;
        cache.putPage(block_offset, data, false);
        page = cache.getPage(block_offset);
    }
    
    size_t to_copy = std::min(count, BLOCK_SIZE - block_index);
    std::memcpy(buf, page->data.data() + block_index, to_copy);
    file->offset += to_copy;
    return to_copy;
}

ssize_t lab2_write(int fd, const void* buf, size_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fd_table[fd]) return -1;
    FileDescriptor* file = fd_table[fd];
    Cache& cache = file->cache;

    off_t block_offset = file->offset / BLOCK_SIZE * BLOCK_SIZE;
    size_t block_index = file->offset % BLOCK_SIZE;

    CachePage* page = cache.getPage(block_offset);
    if (!page) {
        std::vector<char> data(BLOCK_SIZE, 0);
        cache.putPage(block_offset, data, false);
        page = cache.getPage(block_offset);
    }

    size_t to_copy = std::min(count, BLOCK_SIZE - block_index);
    std::memcpy(page->data.data() + block_index, buf, to_copy);
    page->dirty = true;
    file->offset += to_copy;
    return to_copy;
}
