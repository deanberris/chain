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
// cassert -- we're actually going to enforce assertions if we're built in debug
// mode.
#ifndef NDEBUG
#include <cassert>
#else
#define assert(x)
#endif

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

    // We also allow chains to copy from already constructed chains. We require
    // and enforce a noexcept guarantee to make sure we can rely on copy
    // operations to not throw.
    chain_t(chain_t const &other) noexcept;

    // Now we can start implementing assignment since we now already have a
    // defined copy constructor. We also want to enforce that assignment is a
    // copy and swap and that this does not throw exceptions.
    chain_t& operator=(chain_t rhs) noexcept {
      rhs.swap(*this);
      return *this;
    }

    // To be complete with the value semantics of chains we then require a swap
    // member that swaps the contents of two instances. This does *not* change
    // the allocators used by either object and is enforced appropriately if the
    // allocators of this and the other object do not compare equal. The swap
    // member returns whether or not it actually succeeded.
    bool swap(chain_t &other) noexcept {
      // Our preconditions are:
      //   1) That this object actually has a defined allocator.
      //   2) That the other object also actually has a defined allocator.
      assert(allocator_ != nullptr);
      assert(other.allocator_ != nullptr);
      // Here we try and enforce the invariant that we do not throw even if and
      // especially since we have a different allocator from the object we're
      // swapping with.
      if (*other.allocator_ == *allocator_) {
        // Most of the time we're not using stateful allocators and we're just
        // going to actually swap some pointers around. We make this the then
        // case of the if as a hint to the compiler that we're almost pretty
        // sure that this will happen most of the time in most user's code.
        std::swap(this->links_, other.links_);
      } else {
        // Here we then need to create a new chain using this object's allocator
        // so that we make sure we're doing the correct thing by preserving this
        // object's invariants. We also create a temporary for the other chain
        // so that we can properly copy the elements using the other chain's
        // special cloning copy constructor.
        chain_t defaulted_this(this->allocator_);
        chain_t defaulted_that(other.allocator_);
        chain_t temporary_this(this->allocator_);
        chain_t temporary_that(other.allocator_);
        // Because we have the temporaries set up correctly at this time as
        // defaulted chains, we can now start the actual copy of links using
        // different allocators that don't compare equal. This is really
        // important that both operations actually do not throw and both have to
        // succeed before we do any mutation of this object and the other
        // object.
        temporary_this.copy_links(other);
        temporary_that.copy_links(*this);
        // Now we check whether both operations actually succeeded completely.
        if (temporary_this == defaulted_this || temporary_that == defaulted_that) {
          // We got here because the copying of links has actually failed for
          // either case. Here we cannot do anything but actually not do
          // anything. We can then signal the calling function that the swap
          // didn't succeed.
          return false;
        }
        // We've passed the failure test and now we're ready to actually make
        // sure that this object will inherit the temporary_this' links in a
        // non-throwing manner while the other will inherit the temporary_that's
        // links.
        this->links_ = temporary_this.links_;
        other.links_ = temporary_that.links_;
        // WHen we get to this point we're pretty confident that we now have the
        // elements copied and that the swap has actually done what it's
        // supposed to do.
        return true;
      }
    }

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

    // We then provide the inverse of the equivalence relation operator.
    bool operator!=(chain_t const &other) const;

   private:
    Allocator *allocator_;
    // TODO(dberris): make this happen!
    void *links_;

    // The cloning constructor (or actually, cloning function) does an explicit
    // copy of the data in the links using the current object's associated
    // allocator and making sure there are no errors in the whole operation. It
    // gracefully backs out when it cannot copy all the links and cleans up
    // after itself.
    void copy_links(chain_t const &other) noexcept;
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


  // Here we start defining the free functions that we want to find via ADL. We
  // start with the few common implementations for the basic algorithms that
  // enforce value semantics. We then provide a no exception guarantee for this
  // particular implementation.
  template <class Element, class Allocator>
  void swap(chain_t<Element, Allocator> &l, chain_t<Element, Allocator> &r) noexcept {
    l.swap(r);
  }

}  // namespace chain

#endif  // CHAIN_HPP
