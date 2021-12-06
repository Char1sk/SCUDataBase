/**
 * index_iterator.h
 * For range scan of b+ tree
 */
#pragma once
#include "page/b_plus_tree_leaf_page.h"

namespace scudb {

#define INDEXITERATOR_TYPE                                                     \
  IndexIterator<KeyType, ValueType, KeyComparator>

INDEX_TEMPLATE_ARGUMENTS
class IndexIterator {
public:
  // you may define your own constructor based on your member variables
  IndexIterator();
  IndexIterator(B_PLUS_TREE_LEAF_PAGE_TYPE *leaf, int index, BufferPoolManager *bpm);
  ~IndexIterator();

  bool isEnd() {
    return (leaf_ == nullptr) || (index_ >= leaf_->GetSize());
  }

  const MappingType &operator*() {
    return leaf_->GetItem(index_);
  }

  IndexIterator &operator++();

private:
  // add your own private member variables here
  B_PLUS_TREE_LEAF_PAGE_TYPE *leaf_;
  int index_;
  BufferPoolManager *bmp_;
};

} // namespace scudb
