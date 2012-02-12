// Copyright 2012 Dean Michael Berris <dberris@google.com>.
// Copyright 2012 Google, Inc.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// We want to test the semantics of the chain implementation so we include the
// most top-level header that wires the basics of chains up.
#include <chain/chain.hpp>

// We also want to be able to assert that our assumptions and understanding is
// correct.
#include <cassert>

// In the context of this test we would like to define an allocator
// implementation that we can use in the tests for usage.
template <class T>
struct test_allocator {
  // TODO(dberris): Implement this to conform to the Allocator concept
  // requirements.
};

// For the most part we would like to be able to construct a chain. The
// following test describes all the supported constructions of a chain.
void test_construction() {
  // We set up the environment from within the function to work on everything
  // that is defined in the chain namespace.
  using chain::u32chain;
  using chain::u16chain;
  using chain::u8chain;
  using chain::chain;

  // The simplest case we want is to use the supported various predefined chain
  // types. We imply certain things from the type and we'd like to be able to
  // explicitly say what the underlying storage and unit of a member in the
  // chain would be. Here are the default constructions of these types.
  u32chain utf_32;  // Each unit in the chain is 32 bits wide.
  u16chain utf_16;  // Each unit in the chain is 16 bits wide.
  u8chain utf_8;  // Each unit in the chain is 8 bits.
  chain normal;  // Each unit is a char.

  // More complex usages of the chain involve working with existing std::string
  // objects and string literals.
  u32chain sample_32(U"The quick brown fox jumps over the lazy dog.");
  u16chain sample_16(u"The quick brown fox jumps over the lazy dog.");
  unsigned char quick_brown[] = "The quick brown fox!!!";
  u8chain sample_8(quick_brown);
  chain sample_normal("The quick brown fox jumps over the lazy dog.");

  // In the following cases we intend to be able to construct chains from
  // standard string types.
  u32chain sample_string_32(std::u32string(U"Aloha!"));
  u16chain sample_string_16(std::u16string(u"Aloha!"));
  typedef std::basic_string<unsigned char> u8string;
  u8chain sample_string_8(u8string(quick_brown));
  chain sample_string_normal(std::string("Aloha!"));

  {
    using namespace chain;
    // We also want to support custom allocators.
    test_allocator<char32_t> uint32_allocator;
    chain_t<char32_t, test_allocator<char32_t>>
        custom_allocator_32(&uint32_allocator);
    test_allocator<char16_t> uint16_allocator;
    chain_t<char16_t, test_allocator<char16_t>>
        custom_allocator_16(&uint16_allocator);
    test_allocator<unsigned char> uint8_allocator;
    chain_t<unsigned char, test_allocator<unsigned char>>
        custom_allocator_8(&uint8_allocator);
    test_allocator<char> normal_allocator;
    chain_t<char, test_allocator<char>>
        custom_allocator_normal(&normal_allocator);
  }
}

// Not only don't we want chains to be constructible, we also want them to be
// copy constructible. This is important because we intend to provide full value
// semantics.
void test_copy() {
  // When we talk about copy construction, we make sure that the newly
  // constructed chain is semantically equivalent to another chain. This implies
  // content equivalence, not necessarily referential equivalence (in the
  // implementation, they may refer to the same area in memory for the links of
  // the chain, but is not guaranteed because of custom allocator support).
  using chain::chain;
  chain original("The quick brown fox is quick and brown!");
  chain copied(original);
  assert(original == copied);
}

// We want to ensure that as part of the value semantics of the chain objects
// we also allow equivalence through assignment.
void test_assignment() {
  // Assignment should be implemented through copies and should work even if two
  // objects were constructed with different allocators.
  using chain::chain;
  chain original("The quick brown fox jumps quickly!");
  chain another("I don't really care what's in here!");
  another = original;
  assert(original == another);
}

// We also want to ensure as one of the important requirements on objects that
// exhibit value semantics is that we can actually swap two of them.
void test_swap() {
  // TODO(dberris): make this happen!
}

int main(int argc, char *argv[]) {
  // This gives a very coherent and narrative story on what exactly what we
  // would want to use a chain for. We define a usage semantic that is very much
  // akin to how you would normally use std::string objects.
  //
  // Setting the environment up.
  using namespace chain;

  // There are a whole family of tests that we would like to invoke. Here we
  // reference the most common ones we'd like to perform in this usage test. The
  // first set of tests verify that the chain type behaves very much like a
  // value type.
  test_construction();
  test_copy();
  test_assignment();
  test_swap();

  // Once we reach this point we are certain that the usage tests are all good.
  return 0;
}
