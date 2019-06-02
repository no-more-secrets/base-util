/****************************************************************
* Graphs
****************************************************************/
#pragma once

#include "base-util/macros.hpp"
#include "base-util/non-copyable.hpp"
#include "base-util/bimap.hpp"
#include "base-util/misc.hpp"

#include <map>
#include <numeric>
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

    // Returns true if this graph has a cycle in it.
    bool cyclic() const;

protected:

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

    return DirectedGraph<NameT>(
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

// Check is cyclic. FIXME: This algo is VERY SLOW and needs to be
// made more efficient.
template<typename NameT>
bool DirectedGraph<NameT>::cyclic() const {
  // Must be `resize` and not `reserve`.
  std::vector<Id> ids; ids.resize( m_names.size() );
  std::iota( ids.begin(), ids.end(), 0 );
  // First check if any individual nodes have cycles to
  // themselves.
  for( Id id : ids ) {
    auto const& children = m_edges[id];
    if( std::find( children.begin(), children.end(), id )
                != children.end() )
      return true;
  }
  // Now check if, for any pair of nodes in the graph A and B, A
  // is accessible from B and B is accessible from A.
  auto is_less = [this]( Id lhs, Id rhs ) {
    auto children =
        accessible( m_names.val( rhs ), /*with_self=*/false );
    return std::find(
        children.begin(), children.end(), m_names.val( lhs ) )
     != children.end();
  };
  for( Id id1 : ids )
    for( Id id2 : ids )
      if( is_less( id1, id2 ) && is_less( id2, id1 ) )
        return true;
  return false;
}

/****************************************************************
* Directed Acyclic Graph (DAG)
****************************************************************/

// Same as DirectedGraph except it will check (upon construction)
// that the graph is acyclic. If not it will throw. It also has
// some additional methods, such as `sorted()` which make sense
// on DAGs.
template<typename NameT>
class DirectedAcyclicGraph : public DirectedGraph<NameT> {

public:

    template<typename MapT>
    static DirectedAcyclicGraph<NameT> make_dag( MapT const& m );

    // Return a list of nodes sorted in such a way that, if node
    // A is accessible from node B then A will necessarily appear
    // before B in the list (given that this is an acyclic graph,
    // it is always possible to do this for all nodes in the
    // graph). Two nodes where neither is reachable from the
    // other will be in an unspecified order relative to
    // eachother.  E.g., the graph:
    //
    //                  B ---
    //                      |
    //                       >--> A
    //                      |
    //                  C ---
    //
    // when sorted, may yield either {A,B,C} or {A,C,B};
    std::vector<NameT> sorted() const;

    using typename DirectedGraph<NameT>::NamesMap;
    using typename DirectedGraph<NameT>::Id;
    using typename DirectedGraph<NameT>::GraphVec;

private:
    DirectedAcyclicGraph( DirectedGraph<NameT>&& graph );

};

template<typename NameT>
using DAG = DirectedAcyclicGraph<NameT>;

template<typename NameT>
DirectedAcyclicGraph<NameT>::DirectedAcyclicGraph(
    DirectedGraph<NameT>&& graph )
    : DirectedGraph<NameT>( std::move( graph ) )
{
    ASSERT_( !this->cyclic() );
}

template<typename NameT>
template<typename MapT>
DirectedAcyclicGraph<NameT>
DirectedAcyclicGraph<NameT>::make_dag( MapT const& m ) {

    return DirectedAcyclicGraph( make_graph( m ) );
}

// FIXME: efficiency of this method is terrible, only use on
// small graphs. Need to find a proper algorithm.
template<typename NameT>
std::vector<NameT>
DirectedAcyclicGraph<NameT>::sorted() const {
  std::vector<Id> ids; ids.resize( this->m_names.size() );
  std::iota( ids.begin(), ids.end(), 0 );

  // Compare two nodes.
  auto is_less = [this]( Id lhs, Id rhs ) {
    if( lhs == rhs ) return false;
    auto children = this->accessible( this->m_names.val( rhs ),
                                      /*with_self=*/false );
    return std::find(
        children.begin(), children.end(), this->m_names.val( lhs ) )
     != children.end();
  };

  // Standard sorting algos will not work here; this is probably
  // due to the fact that that we can have two nodes A,B that
  // satisfy is_less(A,B) == false && is_less(B,A) == false && (A
  // != B). This then messes up the ability to optimize a sort by
  // performing fewer than N^2 comparisons. Instead, the only way
  // to order the list of nodes is to perform a comparison be-
  // tween each and every pair of nodes. Even bubble sort won't
  // work.
  //
  // /*wont' work*/ std::sort( ids.begin(), ids.end(), is_less );

  // Exhaustive sort.
  for( size_t i = 0; i < ids.size()-1; ++i )
    for( size_t j = i+1; j < ids.size(); ++j )
      if( is_less( ids[j], ids[i] ) )
        std::swap( ids[i], ids[j] );

  std::vector<NameT> res; res.reserve( this->m_names.size() );
  for( Id id : ids )
    res.push_back( this->m_names.val( id ) );
  return res;
}

} // namespace util
