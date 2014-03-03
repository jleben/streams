#ifndef STREAMS_UTILITY_INCLUDED
#define STREAMS_UTILITY_INCLUDED

#include <array>
#include <tuple>

namespace streams {

using std::get;
using std::array;
using std::tuple;
using std::make_tuple;

struct ignore {};

template<size_t I>
const ignore & get( const ignore & input ) { return input; }

template<size_t I, typename T>
struct lace : lace< I, tuple<T> >
{};

template<size_t I, typename Head, typename ...Tail>
struct lace<I, tuple<Head, Tail...> > : lace<I-1, tuple<Head, Head, Tail...> >
{};

template<typename Head, typename ...Tail>
struct lace<1, tuple<Head, Tail...> >
{
  typedef tuple<Head, Tail...> type;
};

template <typename T, typename ...TT>
array<T, sizeof...(TT)+1> make_array( const T & v, const TT ... vv )
{
  return array<T, sizeof...(TT)+1>({v, vv...});
}

namespace detail
{

template<size_t I, typename T>
struct tuplicator
{
  static auto value(const T & v)
  {
    return tuple_cat( tuplicator<I-1,T>::value(v), make_tuple(v) );
  }
};

template<typename T>
struct tuplicator<1,T>
{
  static auto value(const T & v) { return make_tuple(v); }
};

}

template<size_t N, typename T>
auto tuplicate( const T & v )
{
  return detail::tuplicator<N,T>::value(v);
}

//

template<size_t I, typename Array>
struct array_to_tuple
{
  static auto value( const Array & array )
  -> decltype( tuple_cat( array_to_tuple<I-1,Array>::value(array), make_tuple(array[I]) ) )
  {
    return tuple_cat( array_to_tuple<I-1,Array>::value(array), make_tuple(array[I]) );
  }
};

template<typename Array>
struct array_to_tuple<0, Array>
{
  static auto value( const Array & array )
  -> decltype( make_tuple(array[0]) )
  {
    return make_tuple(array[0]);
  }
};

} // namespace streams

#endif // STREAMS_UTILITY_INCLUDED
