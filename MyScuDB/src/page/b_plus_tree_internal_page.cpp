/**
 * b_plus_tree_internal_page.cpp
 */
#include <iostream>
#include <sstream>

#include "common/exception.h"
#include "page/b_plus_tree_internal_page.h"

namespace scudb {
/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/
/*
 * Init method after creating a new internal page
 * Including set page type, set current size, set page id, set parent id and set
 * max page size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Init(page_id_t page_id,
                                          page_id_t parent_id)
{
    SetPageType(IndexPageType::INTERNAL_PAGE);
    SetSize(0);

    SetParentPageId(parent_id);
    SetPageId(page_id);
    //// ????
    int max_size = (PAGE_SIZE - sizeof(BPlusTreeInternalPage)) / sizeof(MappingType) - 1;
    SetMaxSize(max_size);
}
/*
 * Helper method to get/set the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
KeyType B_PLUS_TREE_INTERNAL_PAGE_TYPE::KeyAt(int index) const
{
    // array of MappingType, which is pair<k,v>
    return array[index].first;
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetKeyAt(int index, const KeyType &key)
{
    array[index].first = key;
}

/*
 * Helper method to find and return array index(or offset), so that its value
 * equals to input "value"
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueIndex(const ValueType &value) const
{
    // find linearly
    for (int i = 0; i != GetSize(); ++i) {
        if (value == ValueAt(i)) {
            return i;
        }
    }
    return -1;
}

/*
 * Helper method to get the value associated with input "index"(a.k.a array
 * offset)
 */
INDEX_TEMPLATE_ARGUMENTS
ValueType B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueAt(int index) const
{
    return array[index].second;
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetValueAt(int index, const ValueType& value)
{
    array[index].second = value;
}

/*****************************************************************************
 * LOOKUP
 *****************************************************************************/
/*
 * Find and return the child pointer(page_id) which points to the child page
 * that contains input "key"
 * Start the search from the second key(the first key should always be invalid)
 */
INDEX_TEMPLATE_ARGUMENTS
ValueType
B_PLUS_TREE_INTERNAL_PAGE_TYPE::Lookup(const KeyType &key,
                                       const KeyComparator &comparator) const
{
    // binary search first
    int left = 1;
    int right = GetSize() - 1;
    int mid;
    while (left <= right) {
        mid = left + (right - left) / 2;
        int result = comparator(array[mid].first, key);
        if (result == 0) {
            left = mid;
            break;
        } else if (result < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    int index = left;

    // right most
    if (index >= GetSize()) {
        return array[GetSize() - 1].second;
    }

    if (comparator(array[index].first, key) == 0) {
        // right side
        return array[index].second;
    } else {
        // left side
        return array[index - 1].second;
    }
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Populate new root page with old_value + new_key & new_value
 * When the insertion cause overflow from leaf page all the way upto the root
 * page, you should create a new root page and populate its elements.
 * NOTE: This method is only called within InsertIntoParent()(b_plus_tree.cpp)
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::PopulateNewRoot(
    const ValueType &old_value, const KeyType &new_key,
    const ValueType &new_value)
{
    SetSize(2);
    array[0].second = old_value;
    array[1].first = new_key;
    array[1].second = new_value;
}
/*
 * Insert new_key & new_value pair right after the pair with its value ==
 * old_value
 * @return:  new size after insertion
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_INTERNAL_PAGE_TYPE::InsertNodeAfter(
    const ValueType &old_value, const KeyType &new_key,
    const ValueType &new_value)
{
    // get old pair
    int index = ValueIndex(old_value);
    // move all pairs after old pair
    for (int i = GetSize() - 1; i >= index+1; --i) {
        array[i+1].first = array[i].first;
        array[i+1].second = array[i].second;
    }
    array[index+1].first = new_key;
    array[index+1].second = new_value;

    IncreaseSize(1);
    return GetSize();
}

/*****************************************************************************
 * SPLIT
 *****************************************************************************/
/*
 * Remove half of key & value pairs from this page to "recipient" page
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveHalfTo(
    BPlusTreeInternalPage *recipient,
    BufferPoolManager *buffer_pool_manager)
{
    // index of the last half
    int lastIndex = GetSize() - 1;
    int startIndex = lastIndex / 2 + 1;
    int count = lastIndex - startIndex + 1;
    for (int i = 0; i < count; ++i) {
        recipient->array[i].first = array[i+startIndex].first;
        recipient->array[i].second = array[i+startIndex].second;
    }

    SetSize(startIndex);
    recipient->SetSize(count);

    // set parent_page_id of the moved pages
    // ????
    for (int i = 0; i < recipient->GetSize(); i++) {
        auto page_id = recipient->ValueAt(i);
        auto page = buffer_pool_manager->FetchPage(page_id);
        BPlusTreePage *bp = reinterpret_cast<BPlusTreePage *>(page->GetData());
        bp->SetParentPageId(recipient->GetPageId());
        buffer_pool_manager->UnpinPage(page_id, true);
    }
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyHalfFrom(
    MappingType *items, int size, BufferPoolManager *buffer_pool_manager)
{
    // ?????????
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Remove the key & value pair in internal page according to input index(a.k.a
 * array offset)
 * NOTE: store key&value pair continuously after deletion
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Remove(int index)
{
    for (int i = index; i != GetSize() - 1; ++i) {
        array[i].first = array[i+1].first;
        array[i].second = array[i+1].second;
    }
    IncreaseSize(-1);
}

/*
 * Remove the only key & value pair in internal page and return the value
 * NOTE: only call this method within AdjustRoot()(in b_plus_tree.cpp)
 */
INDEX_TEMPLATE_ARGUMENTS
ValueType B_PLUS_TREE_INTERNAL_PAGE_TYPE::RemoveAndReturnOnlyChild()
{
    IncreaseSize(-1);
    return ValueAt(0);
}
/*****************************************************************************
 * MERGE
 *****************************************************************************/
/*
 * Remove all of key & value pairs from this page to "recipient" page, then
 * update relavent key & value pair in its parent page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveAllTo(
    BPlusTreeInternalPage *recipient, int index_in_parent,
    BufferPoolManager *buffer_pool_manager)
{   //// ??????????????
    Page *page = buffer_pool_manager->FetchPage(GetParentPageId());
    BPInternalPage *parent_page = reinterpret_cast<BPInternalPage *>(page->GetData());

    array[0].first = parent_page->KeyAt(index_in_parent);
    buffer_pool_manager->UnpinPage(GetParentPageId(), false);

    recipient->CopyAllFrom(array, GetSize(), buffer_pool_manager);

    // change parent id
    for (int i = 0; i < GetSize(); ++i) {
        page_id_t child_page_id = ValueAt(i);
        page = buffer_pool_manager->FetchPage(child_page_id);
        BPInternalPage *child_page = reinterpret_cast<BPInternalPage *>(page->GetData());

        child_page->SetParentPageId(recipient->GetPageId());
        buffer_pool_manager->UnpinPage(child_page_id, true);
    }

    buffer_pool_manager->UnpinPage(GetPageId(), true);
    buffer_pool_manager->UnpinPage(recipient->GetPageId(), true);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyAllFrom(
    MappingType *items, int size, BufferPoolManager *buffer_pool_manager)
{
    int startIndex = GetSize();
    MappingType *itemsCopy = items;
    for (int i = 0; i < size; ++i) {
        array[startIndex+i] = *itemsCopy++;
    }
    IncreaseSize(size);
}

/*****************************************************************************
 * REDISTRIBUTE
 *****************************************************************************/
/*
 * Remove the first key & value pair from this page to tail of "recipient"
 * page, then update relavent key & value pair in its parent page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveFirstToEndOf(
    BPlusTreeInternalPage *recipient,
    BufferPoolManager *buffer_pool_manager)
{   //// ????
    MappingType pair{KeyAt(1), ValueAt(0)};
    page_id_t child_page_id = ValueAt(0);
    array[0].second = ValueAt(1);
    Remove(1);

    recipient->CopyLastFrom(pair, buffer_pool_manager);

    auto *page = buffer_pool_manager->FetchPage(child_page_id);
    auto child = reinterpret_cast<BPlusTreePage *>(page->GetData());
    child->SetParentPageId(recipient->GetPageId());

    buffer_pool_manager->UnpinPage(child->GetPageId(), true);
    buffer_pool_manager->UnpinPage(GetPageId(), true);
    buffer_pool_manager->UnpinPage(recipient->GetPageId(), true);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyLastFrom(
    const MappingType &pair, BufferPoolManager *buffer_pool_manager)
{
    auto *page = buffer_pool_manager->FetchPage(GetParentPageId());
    auto parent = reinterpret_cast<BPlusTreeInternalPage *>(page->GetData());

    auto index = parent->ValueIndex(GetPageId());
    auto key = parent->KeyAt(index+1);

    array[GetSize()] = {key, pair.second};
    IncreaseSize(1);
    parent->SetKeyAt(index + 1, pair.first);

    buffer_pool_manager->UnpinPage(parent->GetPageId(), true);
}

/*
 * Remove the last key & value pair from this page to head of "recipient"
 * page, then update relavent key & value pair in its parent page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::MoveLastToFrontOf(
    BPlusTreeInternalPage *recipient, int parent_index,
    BufferPoolManager *buffer_pool_manager)
{
    MappingType last = array[GetSize() - 1];
    IncreaseSize(-1);
    page_id_t child_id = last.second;

    recipient->CopyFirstFrom(last, parent_index, buffer_pool_manager);

    Page *page = buffer_pool_manager->FetchPage(child_id);
    BPInternalPage *child_page = reinterpret_cast<BPInternalPage *>(page->GetData());
    child_page->SetParentPageId(recipient->GetPageId());

    buffer_pool_manager->UnpinPage(child_id, true);
    buffer_pool_manager->UnpinPage(GetPageId(), true);
    buffer_pool_manager->UnpinPage(recipient->GetPageId(), true);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::CopyFirstFrom(
    const MappingType &pair, int parent_index,
    BufferPoolManager *buffer_pool_manager)
{
    Page *page = buffer_pool_manager->FetchPage(GetParentPageId());
    BPInternalPage *parent_page = reinterpret_cast<BPInternalPage *>(page->GetData());

    auto tmp = parent_page->KeyAt(parent_index);
    parent_page->SetKeyAt(parent_index, pair.first);

    InsertNodeAfter(array[0].second, tmp, array[0].second);
    array[0].second = pair.second;

    buffer_pool_manager->UnpinPage(GetParentPageId(), true);
}

/*****************************************************************************
 * DEBUG
 *****************************************************************************/
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::QueueUpChildren(
    std::queue<BPlusTreePage *> *queue,
    BufferPoolManager *buffer_pool_manager) {
  for (int i = 0; i < GetSize(); i++) {
    auto *page = buffer_pool_manager->FetchPage(array[i].second);
    if (page == nullptr)
      throw Exception(EXCEPTION_TYPE_INDEX,
                      "all page are pinned while printing");
    BPlusTreePage *node =
        reinterpret_cast<BPlusTreePage *>(page->GetData());
    queue->push(node);
  }
}

INDEX_TEMPLATE_ARGUMENTS
std::string B_PLUS_TREE_INTERNAL_PAGE_TYPE::ToString(bool verbose) const {
  if (GetSize() == 0) {
    return "";
  }
  std::ostringstream os;
  if (verbose) {
    os << "[pageId: " << GetPageId() << " parentId: " << GetParentPageId()
       << "]<" << GetSize() << "> ";
  }

  int entry = verbose ? 0 : 1;
  int end = GetSize();
  bool first = true;
  while (entry < end) {
    if (first) {
      first = false;
    } else {
      os << " ";
    }
    os << std::dec << array[entry].first.ToString();
    if (verbose) {
      os << "(" << array[entry].second << ")";
    }
    ++entry;
  }
  return os.str();
}

// valuetype for internalNode should be page id_t
template class BPlusTreeInternalPage<GenericKey<4>, page_id_t,
                                           GenericComparator<4>>;
template class BPlusTreeInternalPage<GenericKey<8>, page_id_t,
                                           GenericComparator<8>>;
template class BPlusTreeInternalPage<GenericKey<16>, page_id_t,
                                           GenericComparator<16>>;
template class BPlusTreeInternalPage<GenericKey<32>, page_id_t,
                                           GenericComparator<32>>;
template class BPlusTreeInternalPage<GenericKey<64>, page_id_t,
                                           GenericComparator<64>>;
} // namespace scudb
