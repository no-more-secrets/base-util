/****************************************************************
* Base classes for specifying semantics of derived class
****************************************************************/
#pragma once

namespace util {

// This is a base class used  for  classes  that  should  not  be
// copied but can be move constructed.
struct movable_only {

    movable_only()                                 = default;
    ~movable_only()                                = default;

    movable_only( movable_only const& )            = delete;
    movable_only& operator=( movable_only const& ) = delete;

    movable_only( movable_only&& )                 = default;
    movable_only& operator=( movable_only&& )      = default;

};

// This is a base class used  for  classes  that  should  not  be
// moved but can be copy constructed.
struct non_movable {

    non_movable()                                = default;
    ~non_movable()                               = default;

    non_movable( non_movable const& )            = default;
    non_movable& operator=( non_movable const& ) = default;

    non_movable( non_movable&& )                 = delete;
    non_movable& operator=( non_movable&& )      = delete;

};

// Can neither copy nor move.
struct non_copy_non_move {

    non_copy_non_move()                                      = default;
    ~non_copy_non_move()                                     = default;

    non_copy_non_move( non_copy_non_move const& )            = delete;
    non_copy_non_move& operator=( non_copy_non_move const& ) = delete;

    non_copy_non_move( non_copy_non_move&& )                 = delete;
    non_copy_non_move& operator=( non_copy_non_move&& )      = delete;

};

// Class for singletons to inherit from:  no  copying  or  moving.
struct singleton : public movable_only,
                   public non_movable { };

} // namespace util
