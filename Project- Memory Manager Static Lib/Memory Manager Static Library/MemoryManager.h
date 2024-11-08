#ifndef MEMORYMANAGER_MEMORYMANAGER_H
#define MEMORYMANAGER_MEMORYMANAGER_H

#include <functional>
#include <vector>
#include <iostream>
#include <fcntl.h>   // for open to work
#include <unistd.h>  // to write & close
#include <cstring>   // for memset
#include <climits>   // max funcs make things easier
#include <algorithm> // for findif alg
#include <memory>    // for std::unique_ptr
#include <bitset>
#include <cstddef>
#include <sys/mman.h> // for prot read & write etc
#include <cmath> // ceil func makes things easier too

// discussion tip: use struct for hole and block:
class MemoryManager {
public:
    MemoryManager(unsigned wordSize, std::function<int(int, void *)> allocator);
    ~MemoryManager();
    void initialize(size_t sizeInWords);
    void shutdown();
    void* allocate(size_t sizeInBytes);
    void free(void *address);
    void setAllocator(std::function<int(int, void *)> allocator);
    int dumpMemoryMap(char *filename);
    void *getList();
    void *getBitmap();
    unsigned getWordSize();
    void* getMemoryStart();
    unsigned getMemoryLimit();

private:
    unsigned int _wordSize;
    std::function<int(int, void *)> _allocator;
    void *_block;
    size_t _sizeInWords;
    std::vector<std::vector<int>> _history; // this is my data structure of choice that will keep track of memory holes & blocks
};

int bestFit(int sizeInWords, void *list);
int worstFit(int sizeInWords, void *list);

#endif // MEMORYMANAGER_MEMORYMANAGER_H