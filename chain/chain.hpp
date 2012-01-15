// Copyright 2012 Dean Michael Berris <dberris@google.com>.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef CHAIN_HPP
#define CHAIN_HPP

#include <shared_ptr>
#include <detail/block_links.hpp>

namespace chain {

template <class CharT, class AllocatorT>
class basic_chain {
  detail::block_links<CharT, AllocatorT> links;
  static AllocatorT default_allocator;
 public:
  basic_chain() : tree(nullptr) {}

  basic_chain(string_ref<CharT> && contents)
  : basic_chain(contents, &default_allocator) {}

  basic_chain(string_ref<CharT> const & contents)
  : basic_chain(std::move(contents), &default_allocator) {}

  basic_chain(string_ref<CharT> &&contents, AllocatorT *allocator)
  : links(block_tree<CharT, AllocatorT>::get_block(contents, allocator), allocator)
  {}

  basic_chain(basic_chain const & other) = default;
  basic_chain(basic_chain && other) = default;
  basic_chain& operator=(basic_chain &&c) = default;
  basic_chain& operator=(basic_chain c) = default;

  void swap(basic_chain &other) {
    swap(other.links, this->links);
  }
};

// We use the link operator (^) to denote concatenation of two basic_chains.
// Here we support an operator that takes a const lvalue as the first and second argument.
template <class CharT, class AllocatorT>
basic_chain<CharT, AllocatorT> operator^(basic_chain<CharT, AllocatorT> const &l, basic_chain<CharT, AllocatorT> const &r);

// Here we support an operator that takes an rvalue as the second argument.
template <class CharT, class AllocatorT>
basic_chain<CharT, AllocatorT> operator^(basic_chain<CharT, AllocatorT> const &l, basic_chain<CharT, AllocatorT> &&r);

template <class CharT, class AllocatorT>
void swap(basic_chain<CharT, AllocatorT> &l, basic_chain<CharT, AllocatorT> &r) {
}

// Convenience typedef.
typedef basic_chain<char, std::allocator<char>> chain;

}  // namespace chain

#endif  // CHAIN_HPP
