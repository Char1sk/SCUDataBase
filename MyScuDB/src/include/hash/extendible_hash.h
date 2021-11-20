/*
 * extendible_hash.h : implementation of in-memory hash table using extendible
 * hashing
 *
 * Functionality: The buffer pool manager must maintain a page table to be able
 * to quickly map a PageId to its corresponding memory location; or alternately
 * report that the PageId does not match any currently-buffered page.
 */

#pragma once

#include <cstdlib>
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <memory>
#include <mutex>

#include "hash/hash_table.h"

namespace scudb {

template <typename K, typename V>
class ExtendibleHash : public HashTable<K, V> {
    // bucket used for hash
    struct Bucket
    {
        // how many bits used to hash the bucket
        int localDepth;
        // maximum size of the bucket
        size_t maxSize;
        // place to store data in the bucket
        // should store key and value to reallocate
        // map is much more convenient than vector
        std::map<K,V> data;
        // construct a bucket with depth
        Bucket(int depth = 0, size_t maxsize = 32)
             : localDepth(depth), maxSize(maxsize) {}
    };
    
    
public:
    // constructor
    ExtendibleHash(size_t size);
    // TODO:
    // helper function to generate hash addressing
    // @return: offset of the bucket
    size_t HashKey(const K &key);
    // TODO:
    // helper function to get global & local depth
    int GetGlobalDepth() const;
    int GetLocalDepth(int bucket_id) const;
    int GetNumBuckets() const;
    // TODO:
    // lookup and modifier
    bool Find(const K &key, V &value) override;
    bool Remove(const K &key) override;
    void Insert(const K &key, const V &value) override;
    // HELP:
    // @return: the true hash value of key
    size_t getRealHashKey(const K &key);

private:
    // how many bits used for addressing
    int globalDepth;
    // maximum depth for buckets
    size_t bucketMaxSize;
    // how many buckets
    size_t bucketNumbers;
    // bucket index to bucket pointer
    std::vector<std::shared_ptr<Bucket>> bucketTable;
    // a mutex to be thread safe
    std::mutex mutex;
};
}// namespace scudb
