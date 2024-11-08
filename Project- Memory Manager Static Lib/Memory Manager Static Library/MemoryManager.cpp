#include "MemoryManager.h"
// notes: focus on main functions like initialize, shutdown, allocate, free, getlist, and allocators first
// bestfit & worstfit will not be in memmanager class 
// one of these functions does not have windows support. check the vid and see which that is. 
// remember to type all your ints, as uint16_t, uint32_t etc
// ANY MEMORY LEAK = 0 POINTS   !!!!

// const:
MemoryManager::MemoryManager(unsigned wordSize, std::function<int(int, void*)> allocator)
: _wordSize(wordSize), _allocator(std::move(allocator)), _block(nullptr), _sizeInWords(0) {
    if (_wordSize <= 0) { // added a check here to make sure that word size is valid
        throw std::invalid_argument("wordSize not > 0, invalid");
    }
}

// dest:
MemoryManager::~MemoryManager(){ // note: never call this directly. 
    shutdown(); 
    _history.clear(); // clean up the history here too 
    // avoids memory leaks 
}

// initialize:
void MemoryManager::initialize(size_t sizeInWords) { 
 /* - this is where we first instantiate an array
        - size of array = sizeInWords * wordSize
    - if initialize() is called on an object that already is initialized, call shutdown() and then reinitialize
    - other functions depend on this one and shoudl return the relevant error for the data type
        - void, nullptr, -1 etc */
    _sizeInWords = sizeInWords; // set size of words = member var size in words
    if (_block != nullptr) { // check if the block is valid here 
        shutdown(); // call shutdown and clear mem if it's not, before problems arise
    }
    // EC: try to avoid using "new" : 
    _block = mmap(nullptr, _sizeInWords * _wordSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    // print debug lol:
    // std::cout << "initializing mem of size: " << _sizeInWords << " words of size: " << _wordSize << "\n";
    if (_block == MAP_FAILED) {
        // std::cout << "mmap failed\n";
        _block = nullptr; // return null if we cannot map / initialize the mem
        return;
    }

    // std::cout << "mem allocated here: " << static_cast<void*>(_block) << "\n";
    memset(_block, 0, _sizeInWords * _wordSize); // fill it with zeros initially, since we start with 100% free mem
    _history.clear(); // clear history, we are just starting, should be empty.
}

// shutdown:
/*  - if a block is initialized, clear all the data. 
    - free heap memory 
    - clear the vect 
    - reset all the member variables */
void MemoryManager::shutdown(){
    if (_block != nullptr) { // check if its null again
        munmap(_block, _sizeInWords * _wordSize); // unmap that specified memory as long as its not null 
        // std::cout << "memory unmapped successfully.\n";
    }
    // reset the vars: 
    _block = nullptr; 
    _sizeInWords = 0;
    _history.clear();  // clear memory history too, this is our vect basically 
}

//getlist:
void *MemoryManager::getList() {
    if (_block == nullptr) {
        return nullptr;
    }

    std::vector<uint16_t> vec; // vec will keep track of block
    vec.push_back(0); // start with iniital free block count, starts at 0 (?)

    bool inFreeBlock = false;
    uint16_t freeStart = 0;
    uint16_t freeLength = 0;

    // step thru memory and check if each block is free or allocated alr
    for (size_t i = 0; i < _sizeInWords; i++) {
        bool isAllocated = ((unsigned char *)_block)[i * _wordSize] != 0;

        if (!isAllocated) {
            if (!inFreeBlock) {
                // create a free block
                inFreeBlock = true;
                freeStart = i;
                freeLength = 1;
            } else {
                // continue the current block (if free)
                freeLength++;
            }
        } else {
            if (inFreeBlock) {
                // end of a free block
                inFreeBlock = false;
                vec.push_back(freeStart);
                vec.push_back(freeLength);
                vec[0]++; // ++ count of free blocks in vect
            }
        }
    }

    // what if the last segment is free?:
    if (inFreeBlock) {
        vec.push_back(freeStart);
        vec.push_back(freeLength);
        vec[0]++; // ++ count of free blocks
    }

    // now we convert the vect into to an array
    auto *list = new uint16_t[vec.size()];
    for (size_t i = 0; i < vec.size(); i++) {
        list[i] = vec[i];
    }

    // and then print out the list for debugging
    //std::cout << "Generated memory list (free segments): ";
    // for (size_t i = 0; i < vec.size(); i++) {
        //std::cout << list[i] << " ";
    //}
    //std::cout << std::endl;

    return list;
}

// allocate:
/*  - sizeInBytes is the number of bytes to be allocated 
        - this might not always be a neat mulitple of wordSize
    - will call the allocator function to find out where to allocate the memory
        - therefore will depend on getList and likely need to call it as well
    - should return a pointer somewhere in the memory block to the starting location of the newly allocated space.*/
void *MemoryManager::allocate(size_t sizeInBytes) {
    if (_block == nullptr || sizeInBytes > _sizeInWords * _wordSize) { 
        std::cerr << "allocatoin failed, not enough room or potentailly not initialized(?)\n";
        return nullptr;
    }

    int sizeInWords = (sizeInBytes + _wordSize - 1) / _wordSize;  // remember to round up here for extra / remainder 
    // more print debug: 
    // std::cout << "allocating " << sizeInBytes << " bytes (" << sizeInWords << " words)\n";

    void *list = getList();  // get the list of any/all free memory segments
    if (list == nullptr) { // print debug/ edge case:
        // std::cerr << "getList() returned null. Cannot proceed with allocation.\n";
        return nullptr;
    }

    int start = _allocator(sizeInWords, list);
    delete[] (uint16_t *)list;

    if (start == -1) {
        std::cerr << "no blocks available/ suitable\n";
        return nullptr;
    }

    memset(((char *)_block) + start * _wordSize, 255, sizeInWords * _wordSize);
    _history.push_back({start, sizeInWords});

    // std::cout << "allocaed here: " << static_cast<void*>((char*)_block + start * _wordSize) << "\n";
    return (char *)_block + start * _wordSize;
}

// free:
/*  - 'address' is a pointer somewhere in the memory block that points to the start of the allocated space that needs to be freed. 
    - this address will be an address that was previously returned from allocate
    - allocated space is being freed, so remember to either extend existing holes, or account for new holes that are created here */
void MemoryManager::free(void *address){
    if(_block == nullptr || address == nullptr){
        return; // just another condition check 
    }

    // std::cout << "free memory here: " << address << "\n";
    for (size_t i = 0; i < _history.size(); i++) {
        // std::cout << "checking history block at: " << i << ": Start = " << _history[i][0] << ", Size = " << _history[i][1] << "\n";
        if ((char*)_block + _history[i][0] * _wordSize == address) {
            // std::cout << "found& freeing block at: " << i << "\n";
            memset((char*)_block + _history[i][0] * _wordSize, 0, _history[i][1] * _wordSize);
            _history.erase(_history.begin() + i); // remove from history 
            return;
        }
    }
}

// set alloc:
/*  - this is a setter function
    - it is responsible for changing the member variable to the new allocator */
void MemoryManager::setAllocator(std::function<int(int, void *)> allocator){
    _allocator = std::move(allocator); 
}

// dumpmap (pain):
/*  - this functions should print out the current list of holes to a file. 
    - this MUST use POSIX calls, and we cannot use fstream objects.
    - use  open(filename, O_RDWR | O_CREAT | O_TRUNC, 0777) to create, enable read-write, and truncate the file on creation
    - remember to call close on the file descriptor before ending the function 
         - otherwise, changes may not save properly */
int MemoryManager::dumpMemoryMap(char *filename){
    if(_block == nullptr){
        return -1; // if its null blah blah blah
    }
    
    int _fileDesc = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0777);
    // std::cout << "dumping mmap into this file: " << filename << "\n";
    if (_fileDesc != -1){
        auto *list = (uint16_t *)getList();
        if(*list != 0){
            std::string buffer = std::string();
            buffer += "[" + std::to_string(list[1]) + ", " + std::to_string(list[2]) + "]";

            for(int i = 1; i < *list; i ++){
                buffer += " - [" + std::to_string(list[2 * i + 1]) + ", " + std::to_string(list[2 * i + 2]) + "]";
            }
            write(_fileDesc, buffer.c_str(), buffer.size());
        }
        delete[] list;
        close(_fileDesc);
    }
    
    return _fileDesc;
}

// bitmap (PAIN):
/*- this should use std::bitset
    - this should return a pointer to the start of an array of 1-byte type that represents the current blocks and holes.
        - should use 1s and 0s for each words.
        - 0 = hole, 1 = block
    - first two bytes in the bitstream should represent the size of the array, in LITTLE ENDIAN format
    for ex: 
        - given this list of holes: [0,10]-[12,2]-[20,6]
        - in words, it will look like: 
             00000000 00110011 11110000 00
        - there are 26 words here, but each byte only stores 8 bits. we will need an extra byte for the last 2: 
             00000000 00110011 11110000 00000000
        - now, we need to mirror each byte individually: 
            00000000 11001100 00001111 00000000
        - now, we add the two size bytes to the front. since we have 4 bytes above, the size is 4: 
            00000000 00000010
        - but when we flip to little endian, it looks like: 
             00000010 00000000 
        - now, we can assemble the complete array: 
             00000010 00000000 00000000 11001100 00001111 00000000
        - in integer form, this will look like: [4, 0, 0, 204, 15, 0]
        - we will return a pointer to the beginning of this array  */
void *MemoryManager::getBitmap() {
    // first we check if the memory block is uninitialized (null)
    if (_block == nullptr) {
        return nullptr;
    }

    // then we can calculate the number of bytes needed for the bitmap representation
    int size = ceil(static_cast<double>(_sizeInWords) / 8.0);
    uint8_t* bitmap = new uint8_t[size];
    memset(bitmap, 0, size); // Initialize all bytes in the bitmap to 0

    // and then we iterate through each word in the memory block
    for (int i = 0; i < _sizeInWords; i++) {
        // we can use a bool to check if the current word is allocated (non-zero)
        bool isAllocated = ((unsigned char*)_block)[i * _wordSize] != 0;

        // calculate the byte index & bit position within the byte
        int byteIndex = i / 8;
        int bitIndex = i % 8;

        // set corresponding bit in the bitmap if the word is allocated
        if (isAllocated) {
            bitmap[byteIndex] |= (1 << bitIndex);
        }
    }

    // create an array for the final output (size bytes + 2 bytes for length prefix)
    uint8_t* byteArray = new uint8_t[size + 2];

    // then store the size of the bitmap in the first two bytes (little-endian format)
    byteArray[0] = static_cast<uint8_t>(size & 0xFF);
    byteArray[1] = static_cast<uint8_t>((size >> 8) & 0xFF);

    // copy the bitmap data into the array
    for (int i = 0; i < size; i++) {
        byteArray[i + 2] = bitmap[i];
    }

    // free the temporary bitmap memory
    delete[] bitmap;

    // & finally we can return the array as a void pointer
    return static_cast<void*>(byteArray);
}

// getters: 
unsigned MemoryManager::getWordSize() { return _wordSize; }

void *MemoryManager::getMemoryStart() { return _block; }

unsigned MemoryManager::getMemoryLimit() { return _sizeInWords * _wordSize; }

// bestfit:
// remember: the "best fit" is the smallest hole that will fit the requirements
int bestFit(int sizeInWords, void *list) {
    auto *_list = (uint16_t *)list;
    if (_list[0] == 0) {
        return -1; // if there's no free blocks available
    }

    int minIndex = -1;
    int minSize = INT_MAX; // 

    for (int i = 1; i <= _list[0]; i++) {
        int startIndex = _list[2 * i - 1];
        int blockSize = _list[2 * i];

        if (blockSize >= sizeInWords && blockSize < minSize) {
            minIndex = startIndex;
            minSize = blockSize;
        }
    }

    if (minIndex == -1) {
        return -1; // if there's no block found that works
    }
    return minIndex;
}

// worstfit:
// remember: "worst fit" is the biggest hole
int worstFit(int sizeInWords, void *list) {
    auto *_list = (uint16_t *)list;

    // check for if the list is empty, return -1
    if (*_list == 0) {
        return -1;
    }

    // find the largest hole in list first 
    int maxIndex = -1;
    int maxSize = -1;

    for (int i = 1; i <= *_list; i++) {
        int startIndex = _list[2 * i - 1];
        int blockSize = _list[2 * i];
        if (blockSize >= sizeInWords && blockSize > maxSize) {
            maxIndex = startIndex;
            maxSize = blockSize;
        }
    }

    return (maxIndex == -1) ? -1 : maxIndex; // return that largest hole that fits/ 
}