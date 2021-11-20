/**
 * LRU implementation
 */
#include <iostream>
#include "buffer/lru_replacer.h"
#include "page/page.h"

namespace scudb {

template <typename T> LRUReplacer<T>::LRUReplacer() {}

template <typename T> LRUReplacer<T>::~LRUReplacer() {}

/*
 * Insert value into LRU
 * tail is the most recently used
 * head is the least recently used
 */
template <typename T> void LRUReplacer<T>::Insert(const T &value)
{
    std::lock_guard<std::mutex> guard(mutex);
    // std::cout << value << std::endl;
    noMutexErase(value);
    // std::cout << value << std::endl;
    dlist.push_back(value);
    // std::cout << value << std::endl;
}

/* If LRU is non-empty, pop the head member from LRU to argument "value", and
 * return true. If LRU is empty, return false
 */
template <typename T> bool LRUReplacer<T>::Victim(T &value)
{
    std::lock_guard<std::mutex> guard(mutex);
    if (dlist.empty())
    {
        return false;    
    }
    value = dlist.front();
    dlist.pop_front();
    return true;
}

/*
 * Remove value from LRU. If removal is successful, return true, otherwise
 * return false
 */
template <typename T> bool LRUReplacer<T>::Erase(const T &value)
{
    std::lock_guard<std::mutex> guard(mutex);
    
    return noMutexErase(value);
}

template <typename T> size_t LRUReplacer<T>::Size()
{
    std::lock_guard<std::mutex> guard(mutex);
    return dlist.size();
}

template <typename T> bool LRUReplacer<T>::noMutexErase(const T &value)
{
    auto it = std::find(dlist.begin(), dlist.end(), value);
    if (it == dlist.end())
    {
        return false;
    }
    else
    {
        dlist.erase(it);
        return true;
    }
}

template class LRUReplacer<Page *>;
// test only
template class LRUReplacer<int>;

} // namespace scudb
