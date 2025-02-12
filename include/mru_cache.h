// #ifndef MRU_CACHE_H
// #define MRU_CACHE_H

// #include <list>
// #include <unordered_map>

// class MRUCache {
// public:
//     explicit MRUCache(size_t capacity);
//     void accessPage(int page);

// private:
//     struct Node {
//         int page;
//         explicit Node(int p) : page(p) {}
//     };

//     std::list<Node> cacheList;
//     std::unordered_map<int, std::list<Node>::iterator> cacheMap;
//     size_t capacity;
// };

// #endif // MRU_CACHE_H
