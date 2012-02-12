// Copyright 2012 Dean Michael Berris <dberris@google.com>.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// chain.hpp
//
#ifndef CHAIN_HPP
#define CHAIN_HPP

// The chain object relies and supports a few standard library types from which
// it's intended to operate well with. We list them down here:
//
// string -- because we want to be able to support different kinds of strings.
#include <string>
// memory -- because we use the standard allocators.
#include <memory>

// We define a namespace for which the whole library implementation will live
// in. We claim the chain namespace to make it obvious that chain types and
// operations live in this namespace.
namespace chain {

  // The heart of the whole library is a type that acts as a container for an
  // immutable chain. We liken the process of creating a chain to forging steel
  // links which are then treated and let to cool concatenated together to form
  // a very robust and largely treated as a single unit that cannot be changed.
  template <class Element, class Allocator>
  struct chain_t {
    // Chains can be constructed in one of the following means.
    //
    // The default construction mechanism initializes a chain to point to
    // nothing. There's a difference between a chain that points to nothing and
    // a chain that is empty.
    chain_t();
    
    // You should also be able to construct a chain from a string literal. The
    // guarantee is that once the chain is constructed from the literal, that
    // the chain will be the same as another chain constructed from the same
    // literal.
    template <int N>
    chain_t(Element const (&literal)[N]);

    // We then support constructing from std::string types whose value types are
    // the same as the element type in this chain, and uses the same allocator.
    template <class Traits>
    chain_t(std::basic_string<Element, Traits, Allocator> const &string);

    // We also support default construction that takes a pointer to an
    // allocator. This is almost equivalent to a default constructed chain where
    // there is a difference between a chain that points to nothing and a chain
    // that is empty.
    explicit chain_t(Allocator *allocator);

    // We also allow chains to copy from already constructed chains.
    chain_t(chain_t const &other);

    // We then move on to defining semantics of various relational opertors on
    // chains.
    //
    // First we ought to be able to compare whether two chains are equivalent.
    // Equivalence implies element-wise equivalance if and only if:
    //   1) The allocators compare equal.
    //   2) The chain refers to the same links.
    // If either of the above is false we fall back into checking element-wise
    // equality.
    // TODO(dberris): Think through whether this is exception safe.
    bool operator==(chain_t const &other) const;
  };


  // For convenience purposes we're defining a few aliases to commonly used
  // chain types.
  //
  // We have a chain that contains char32_t elements using the standard
  // allocator.
  typedef chain_t<char32_t, std::allocator<char32_t>> u32chain;
  // We have a chain that contains char16_t elements using the standard
  // allocator.
  typedef chain_t<char16_t, std::allocator<char16_t>> u16chain;
  // We have a chain that contains unsigned char elements using the standard
  // allocator.
  typedef chain_t<unsigned char, std::allocator<unsigned char>> u8chain;
  // We have a chain that uses the default char type.
  typedef chain_t<char, std::allocator<char>> chain;

}  // namespace chain

#endif  // CHAIN_HPP
