#include "include/сache.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>

// **Реализация конструктора Cache**
Cache::Cache(size_t capacity) : capacity_(capacity), fd_(-1) {}

// **Получение страницы из кеша**
CachePage* Cache::getPage(off_t offset) {
    auto it = page_map_.find(offset);
    if (it != page_map_.end()) {
        cache_list_.splice(cache_list_.begin(), cache_list_, it->second);
        return &*(it->second);
    }
    return nullptr;
}

// **Добавление страницы в кеш**
void Cache::putPage(off_t offset, const std::vector<char>& data, bool dirty) {
    if (cache_list_.size() >= capacity_) {
        evict();
    }
    cache_list_.emplace_front(CachePage{offset, data, dirty});
    page_map_[offset] = cache_list_.begin();
}

// **Удаление страницы из кеша (MRU-алгоритм)**
void Cache::evict() {
    if (!cache_list_.empty()) {
        auto& page = cache_list_.front();  // Удаляем **самую недавно использованную** страницу
        if (page.dirty && fd_ != -1) {
            pwrite(fd_, page.data.data(), BLOCK_SIZE, page.offset);
        }
        page_map_.erase(page.offset);
        cache_list_.pop_front();  // Удаление MRU-страницы
    }
}

// **Установка файлового дескриптора**
void Cache::setFileDescriptor(int fd) {
    fd_ = fd;
}

// -----------------------------------------------
// **Реализация MRUCache**
// -----------------------------------------------

// **Конструктор MRUCache**
MRUCache::MRUCache(size_t capacity) : capacity(capacity) {}

// **Доступ к странице (обновление MRU)**
void MRUCache::accessPage(int page) {
    if (cacheMap.find(page) != cacheMap.end()) {
        cacheList.erase(cacheMap[page]);
    }
    cacheList.push_front(Node(page));
    cacheMap[page] = cacheList.begin();

    if (cacheList.size() > capacity) {
        int lastPage = cacheList.front().page;
        cacheMap.erase(lastPage);
        cacheList.pop_front();
    }
}
