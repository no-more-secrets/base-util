/****************************************************************
* String utilities template implementations
****************************************************************/
#pragma once

namespace util {

/****************************************************************
* To-String utilities
*
* util::to_string  family of overloaded functions are intended so
* that  a  user  can call them on any commonly-used type and they
* will return a sensibly  formatted result. Unlike std::to_string
* these overloads work on  various  containers  as  well, such as
* vectors and tuples. For simple  numeric  types  util::to_string
* delegates  to  std::to_string.  If  all else fails, the default
* overload  attempts  to use a string stream to do the conversion.
*
* See the special note below  on  the  std::string  overload.  In
* short, Whenever the to_string methods  convert a string (or any
* string-like entity) to a string, they will insert quotes in the
* string itself.
****************************************************************/

// Simply delegate to the wrapped type.
template<typename T>
std::string to_string( Ref<T> const& rw ) {
    return util::to_string( rw.get() );
}

// Not  sure if this one is also needed, but doesn't seem to hurt.
template<typename T>
std::string to_string( CRef<T> const& rw ) {
    return util::to_string( rw.get() );
}

template<typename T>
std::string to_string( std::optional<T> const& opt ) {
    return opt ? util::to_string( *opt )
               : std::string( "nullopt" );
}

// This function exists for the purpose of  having  the  compiler
// deduce the Indexes variadic integer arguments that we can then
// use to index the tuple; it probably is not useful to call this
// method  directly  (it is called by to_string). Was not able to
// find a more elegant way of unpacking an arbitrary tuple passed
// in as an argument apart from using  this  helper  function  in-
// volving the index_sequence.
template<typename Tuple, size_t... Indexes>
StrVec tuple_elems_to_string(
        Tuple const& tp,
        std::index_sequence<Indexes...> /*unused*/ ) {
    StrVec res; res.reserve( std::tuple_size_v<Tuple> );
    // Unary right fold of template parameter pack.
    ((res.push_back( util::to_string( std::get<Indexes>( tp ) ))), ...);
    return res;
}

// Will do JSON-like notation. E.g. (1,2,3)
template<typename... Args>
std::string to_string( std::tuple<Args...> const& tp ) {
    auto is = std::make_index_sequence<sizeof...(Args)>();
    auto v = tuple_elems_to_string( tp, is );
    return "(" + join( v, "," ) + ")";
}

// This function exists for the purpose of  having  the  compiler
// deduce the Indexes variadic integer arguments that we can then
// use  to  index  the variant; it probably is not useful to call
// this method directly (it is called by to_string).
template<typename Variant, size_t... Indexes>
std::string variant_elems_to_string(
        Variant const& v,
        std::index_sequence<Indexes...> /*unused*/ ) {
    std::string res;
    // Unary right fold of template parameter pack.
    (( res += (Indexes == v.index())
            ? util::to_string( std::get<Indexes>( v ) ) : ""
    ), ...);
    return res;
}

template<typename... Args>
std::string to_string( std::variant<Args...> const& v ) {
    auto is = std::make_index_sequence<sizeof...(Args)>();
    return variant_elems_to_string( v, is );
}

// Will do JSON-like notation. E.g. (1,"hello")
template<typename U, typename V>
std::string to_string( std::pair<U, V> const& p ) {
    return "(" + util::to_string( p.first )  + ","
               + util::to_string( p.second ) + ")";
}

// Prints in JSON style notation. E.g. [1,2,3]
template<typename T>
std::string to_string( std::vector<T> const& v ) {

    std::vector<std::string> res( v.size() );
    // We  need  this lambda to help std::transform with overload
    // resolution of to_string.
    auto f = []( T const& e ){ return util::to_string( e ); };
    std::transform( std::begin( v   ), std::end( v ),
                    std::begin( res ), f );
    return "[" + join( res, "," ) + "]";
}

template<typename T>
std::ostream& operator<<( std::ostream&         out,
                          std::vector<T> const& v ) {
    return (out << util::to_string( v ));
}

template<typename U, typename V>
std::ostream& operator<<( std::ostream&          out,
                          std::pair<U, V> const& p ) {
    return (out << util::to_string( p ));
}

}
