/****************************************************************
* Graphs
****************************************************************/
#pragma once

#include "base-util/macros.hpp"
#include "base-util/non-copyable.hpp"
#include "base-util/bimap.hpp"
#include "base-util/misc.hpp"

#include <map>
#include <unordered_map>
#include <vector>

namespace util {

/****************************************************************
* Directed Graph (not acyclic)
****************************************************************/

template<typename NameT>
class DirectedGraph : util::movable_only {

public:

    template<
        typename NameT_,
        // typename... to allow for maps that may have additional
        // template parameters (but  which  we  don't  care about
        // here).
        template<
            typename Key,
            typename Val,
            typename...
        >
        typename MapT
    >
    friend DirectedGraph<NameT_> make_graph(
               MapT<
                   NameT_,
                   std::vector<NameT_>
               >
               const& m
    );

    // By default the node with the given name, if found, will be
    // included among the results,  unless  with_self == false in
    // which case it will be left out.
    std::vector<NameT> accessible( NameT const& name,
                                   bool with_self = true ) const;

private:

    using NamesMap = BDIndexMap<NameT>;
    using Id       = size_t;
    using GraphVec = std::vector<std::vector<Id>>;

    DirectedGraph( GraphVec&& edges, NamesMap&& names );

    NamesMap m_names;
    GraphVec m_edges;

};

template<typename NameT>
DirectedGraph<NameT>::DirectedGraph( GraphVec&& edges,
                                     NamesMap&& names )
    : m_names( std::move( names ) ),
      m_edges( std::move( edges ) )
{
    ASSERT_( m_names.size() == m_edges.size() );
}

template<
    typename NameT,
    // typename...  to  allow  for  maps that may have additional
    // template parameters (but which  we  don't care about here).
    template<
        typename Key,
        typename Val,
        typename...
    >
    typename MapT
>
DirectedGraph<NameT> make_graph( MapT<
                                     NameT,
                                     std::vector<NameT>
                                 > const& m ) {

    std::vector<NameT> names; names.reserve( m.size() );
    for( auto const& p : m )
        names.push_back( p.first );
    std::sort( std::begin( names ), std::end( names ) );

    // true == items are sorted, due to above.
    auto bm = BDIndexMap( std::move( names ), true );

    typename DirectedGraph<NameT>::GraphVec edges;
    edges.reserve( m.size() );

    for( size_t i = 0; i < bm.size(); ++i ) {
        auto const& vs = util::get_val( m, bm.val( i ) );
        std::vector<typename DirectedGraph<NameT>::Id> ids;
        ids.reserve( vs.size() );
        for( auto const& v : vs )
            ids.push_back( bm.key( v ) );
        edges.emplace_back( std::move( ids ) );
    }

    return DirectedGraph(
            std::move( edges ), std::move( bm ) );
}

template<typename NameT>
std::vector<NameT>
DirectedGraph<NameT>::accessible( NameT const& name,
                                  bool with_self ) const {
    std::vector<NameT> res;
    std::vector<Id>    visited( m_names.size(), 0 );
    std::vector<Id>    to_visit;

    auto start = m_names.key_safe( name );
    Id self = -1; // invalid Id
    if( start ) {
        self = *start;
        to_visit.push_back( self );
    }
     
    while( !to_visit.empty() ) {
        Id i = to_visit.back(); to_visit.pop_back();
        if( visited[i] > 0 )
            // We  may have duplicates in the stack if we include
            // some  thing  a  second  time  before the first one
            // (which is already in the stack) is visited.
            continue;
        visited[i] = 1;
        auto name = m_names.val( i );
        if( i != self || with_self )
            // Always add nodes that are not  the  starting  node,
            // and then only add the starting node  if  with_self
            // is true (i.e., caller wants it added).
            res.push_back( name );
        for( Id i : m_edges[i] )
            if( visited[i] == 0 )
                to_visit.push_back( i );
    }

    return res;
}

} // namespace util
