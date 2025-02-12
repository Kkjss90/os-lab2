#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <list>
#include <unordered_map>
#include <unistd.h>

constexpr size_t BLOCK_SIZE = 4096;
constexpr size_t CACHE_COUNT = 16;
constexpr size_t MAX_OPEN_FILES = 256;

// **Структура страницы кеша**
struct CachePage {
    off_t offset;
    std::vector<char> data;
    bool dirty;
};

// **Класс Cache (Файловый кеш)**
class Cache {
public:
    explicit Cache(size_t capacity);
    CachePage* getPage(off_t offset);
    void putPage(off_t offset, const std::vector<char>& data, bool dirty);
    void evict();
    void setFileDescriptor(int fd);

private:
    size_t capacity_;
    int fd_;
    std::list<CachePage> cache_list_;
    std::unordered_map<off_t, std::list<CachePage>::iterator> page_map_;
};

// **Класс MRUCache (кеширование по MRU-алгоритму)**
class MRUCache {
public:
    explicit MRUCache(size_t capacity);
    void accessPage(int page);

private:
    struct Node {
        int page;
        explicit Node(int p) : page(p) {}
    };

    std::list<Node> cacheList;
    std::unordered_map<int, std::list<Node>::iterator> cacheMap;
    size_t capacity;
};

#endif // CACHE_H
