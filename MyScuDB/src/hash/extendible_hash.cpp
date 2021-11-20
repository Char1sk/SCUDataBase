#include <list>
#include <iostream>

#include "hash/extendible_hash.h"
#include "page/page.h"

namespace scudb {

/*
 * constructor
 * array_size: fixed array size for each bucket
 */
template <typename K, typename V>
ExtendibleHash<K, V>::ExtendibleHash(size_t size)
     : globalDepth(0), bucketMaxSize(size), bucketNumbers(0)
{
    bucketTable.push_back(std::make_shared<Bucket>(0, size));
}

/*
 * helper function to calculate the hashing address of input key
 * only use lower 'globalDepth' bits
 */
template <typename K, typename V>
size_t ExtendibleHash<K, V>::HashKey(const K &key)
{
    // std::cout << getRealHashKey(key) << std::endl
    //           << ((sizeof(size_t) << 3) - globalDepth) << std::endl
    //           << (getRealHashKey(key) >> ((sizeof(size_t) << 3) - globalDepth)) << std::endl;
    // return getRealHashKey(key) >> ((sizeof(size_t) << 3) - globalDepth);
    return getRealHashKey(key) & ((1 << globalDepth) - 1);
}

/*
 * helper function to return global depth of hash table
 * NOTE: you must implement this function in order to pass test
 */
template <typename K, typename V>
int ExtendibleHash<K, V>::GetGlobalDepth() const
{
    return globalDepth;
}

/*
 * helper function to return local depth of one specific bucket
 * NOTE: you must implement this function in order to pass test
 */
template <typename K, typename V>
int ExtendibleHash<K, V>::GetLocalDepth(int bucket_id) const
{
    return bucketTable[bucket_id]->localDepth;
}

/*
 * helper function to return current number of bucket in hash table
 */
template <typename K, typename V>
int ExtendibleHash<K, V>::GetNumBuckets() const
{
    return bucketNumbers;
}

/*
 * lookup function to find value associate with input key
 */
template <typename K, typename V>
bool ExtendibleHash<K, V>::Find(const K &key, V &value)
{
    std::lock_guard<std::mutex> mguard(mutex);
    
    size_t bindex = HashKey(key);
    auto pbucket = bucketTable[bindex];
    // if pointer exists and bucket has this key
    if (pbucket && pbucket->data.find(key) != pbucket->data.end())
    {
        value = pbucket->data[key];
        return true;
    }
    return false;
}

/*
 * delete <key,value> entry in hash table
 * Shrink & Combination is not required for this project
 */
template <typename K, typename V>
bool ExtendibleHash<K, V>::Remove(const K &key)
{
    std::lock_guard<std::mutex> mguard(mutex);
    
    size_t bindex = HashKey(key);
    std::shared_ptr<Bucket> pbucket = bucketTable[bindex];

    if (!pbucket || pbucket->data.find(key) == pbucket->data.end())
    {
        return false;
    }
    pbucket->data.erase(key);
    return true;
}

/*
 * insert <key,value> entry in hash table
 * Split & Redistribute bucket when there is overflow and if necessary increase
 * global depth
 */
template <typename K, typename V>
void ExtendibleHash<K, V>::Insert(const K &key, const V &value)
{
    // std::cout << key << std::endl;
    // std::cout << value << std::endl;
    std::lock_guard<std::mutex> mguard(mutex);
    
    while (true)
    {
            
        size_t bindex = 0;
        if (globalDepth != 0)
            bindex = HashKey(key);
        std::shared_ptr<Bucket> pbucket = bucketTable[bindex];
        // if the bucket has space, insert success
        if (pbucket->data.size() < bucketMaxSize)
        {
            pbucket->data[key] = value;
            break;
        }
        else
        {
            // split original bucket
            // split it according to its new bit
            std::shared_ptr<Bucket> zeroBucket = std::make_shared<Bucket>(pbucket->localDepth+1, bucketMaxSize);
            std::shared_ptr<Bucket> oneBucket  = std::make_shared<Bucket>(pbucket->localDepth+1, bucketMaxSize);
            for (auto &kvPair : pbucket->data)
            {
                // size_t newHashAddress = HashKey(kvPair.first);
                size_t newRealHash = getRealHashKey(kvPair.first);
                // if new bit is 1
                if (newRealHash & (1 << pbucket->localDepth))
                {
                    oneBucket->data.insert(kvPair);
                }
                else
                {
                    zeroBucket->data.insert(kvPair);                        
                }
            }
            // if the depth equals
            // need to double the table
            if (globalDepth == pbucket->localDepth)
            {
                ++globalDepth;
                size_t osize = bucketTable.size();
                for (size_t i = 0; i < osize; i++) {
                    bucketTable.push_back(bucketTable[i]);
                }
                bucketNumbers *= 2;
            }
            // change items with the new bit
            for (size_t i = 0; i < bucketTable.size(); i++)
            {
                if (bucketTable[i] == pbucket)
                {
                    if (i & (1 << pbucket->localDepth))
                    {
                        bucketTable[i] = oneBucket;
                    }
                    else
                    {
                        bucketTable[i] = zeroBucket;
                    }
                }
            }
        }
        
    }
}

/*
 * get REAL hash value of the key
 */
template <typename K, typename V>
size_t ExtendibleHash<K, V>::getRealHashKey(const K &key)
{
    // use std::hash, and then do hashing
    return std::hash<K>()(key);
}


template class ExtendibleHash<page_id_t, Page *>;
template class ExtendibleHash<Page *, std::list<Page *>::iterator>;
// test purpose
template class ExtendibleHash<int, std::string>;
template class ExtendibleHash<int, std::list<int>::iterator>;
template class ExtendibleHash<int, int>;
} // namespace scudb
