// Copyright 2012 Dean Michael Berris <dberris@google.com>.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef DETAIL_BLOCK_LINKS_HPP
#define DETAIL_BLOCK_LINKS_HPP

namespace chain {

namespace detail {

template <class CharT, class AllocatorT>
class block {
  AllocatorT *allocator;
  CharT *page;
  size_t filled;
  block<CharT, AllocatorT> *next, *previous;
  size_t refcount;

  block(AllocatorT *allocator, CharT *page, size_t filled,
        block<CharT, AllocatorT> *next, block<CharT, AllocatorT> *previous,
        size_t refcount)
  : allocator(allocator), page(page), filled(filled), next(next)
  , previous(previous), refcount(refcount)
  {}

 public:
  static std::tuple<block<CharT, AllocatorT> *, size_t, size_t>
  get_block(string_ref<CharT> && contents, AllocatorT *allocator) {
    static block<CharT, AllocatorT> root{allocator, nullptr, 0, nullptr, nullptr, 1};
    static block<CharT, AllocatorT> *last_block = &root;
    static size_t const page_size = getpagesize();
    assert(page_size && "We need a valid page size that's greater than 0.");
    // TODO(dberris): Make this thread-safe or document as not thread-safe.
    // TODO(dberris): Explore memoization or hashing of contents to conserve blocks.
    // Compute how many blocks you need given the length of the contents and
    // then build the list of blocks appropriately returned.
    block<CharT, AllocatorT> *current_block = last_block;
    while (current_block->filled == page_size && current_block->next != nullptr) {
      current_block = current_block->next;
    }
    block<CharT, AllocatorT> *returned_block = current_block;
    size_t offset = current_block->filled;
    string_ref<CharT>::const_iterator begin = contents.begin(),
        end = contents.end();
    for (int numblocks = (contents.size() / page_size) 
         + (contents.size() % page_size > 0);
         numblocks > 0; --numblocks) {
      string_ref<CharT>::const_iterator segment_end = begin;
      std::advance(segment_end, std::min(page_size, std::distance(begin, end)));
      current_block->page = allocator.allocate(page_size);
      std::copy(begin, segment_end, current_block->page);
      begin = segment_end;
      current_block->filled += std::distance(begin, segment_end);
      current_block->next = new (std::nothrow) block<CharT, AllocatorT>{
        allocator, nullptr, 0, nullptr, current_block, 1};
      current_block = current_block->next;
    }
    last_block = current_block;
    return {returned_block, offset, contents.size()};
  }

  ~block() {
    assert(refcount == 0 && "Deleting a referenced block!");
    allocator->destroy(page);
    allocator->deallocate(page);
    page = nullptr;
    if (previous != nullptr) {
      previous->next = next;
    }
    if (next != nullptr) {
      next->previous = previous;
    }
  }
};


template <class CharT, class AllocatorT>
class block_links {
  typedef tuple<block<CharT, AllocatorT>*, size_t, size_t>> block_offset_length_tuple;
  std::deque<block_offset_length_tuple> links;
 public:
  explicit block_links(block_offset_length_tuple &&t)
  : links{} {
    // Let's determine the correct lengths for each block-offset-length given the
    // results from get_block(...).
    size_t &total_length = get<2>(t);
    size_t &initial_block_offset = get<1>(t);
    block<CharT, AllocatorT> *&current_block;
    while (current_block != nullptr && total_length) {
      links.push_back({
        current_block,
        initial_block_offset,
        total_length -= current_block->filled - initial_block_offset
      });
      initial_block_offset = 0;  // We then always refer to the beginning of the next block.
      current_block = current_block->next;
    }
    assert(current_block != nullptr && !total_length && "We've been given a bogus tuple.");
  }

  // The fundamental operation for block_links is appending of blocks. The links can only
  // grow but we can't really shrink them, except in a subscript operation that creates
  // a new chain.
  void append(block_offset_length_tuple &&t) {
    block<CharT, AllocatorT> *&current_block = get<0>(t);
    if (get<0>(links.back()) == current_block) {
      // Since the last link and the one being appended point to the same block, we just
      // modify the length parameter. This is so that we conserve the space needed to
      // both refer to the same block.
      get<2>(links.back()) += get<2>(t);
      // We also increase the refcount to the block.
      ++current_block->refcount;
    }
  }

  // The slicing operation on the other hand does block offset length calculus instead.
  // This operation only modifies the links this current links container contains. The
  // operands denote the new beginning of the links to use, and the new length.
  void slice(size_t offset, size_t length) {
    for (block_offset_length_tuple &t : links) {
      block<CharT, AllocatorT> *&current_block = get<0>(t);
      size_t &block_offset = get<1>(t);
      size_t &block_length = get<2>(t);
      tie(current_block, block_offset, block_length) = t;
      assert(current_block != nullptr && "Gone off the rails.");
      if (offset > block_length) {
        if (!--current_block->refcount) {
          // Refcount is 0 for the block so we delete it.
          delete current_block;
        }
        links.pop_front();
        offset -= block_length;
      } else {
        if (offset) {
          block_offset += offset;
          block_length -= offset;
          offset = 0;
        } else {
          if (length < block_length) {
            block_length -= length;
            if (block_length == 0) {
              if (!--current_block->refcount) {
                delete current_block;
              }
              links.pop_back();
            }
            break;
          } else {
            length -= block_length;
          }
        }
      }
    }
    assert(!offset && !length && "Invalid offset and length parameters.")
  }

  ~block_links() {
    for (auto i = links.begin(); i != links.end(); ++i) {
      block<CharT, AllocatorT> *&current_block = get<0>(*i);
      if (!--current_block->refcount) delete current_block;
    }
  }

};

}  // namespace detail

}  // namespace chain

#endif  // DETAIL_BLOCK_LINKS_HPP
