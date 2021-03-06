#ifndef STREAMS_EXPERIMENTAL_INCLUDED
#define STREAMS_EXPERIMENTAL_INCLUDED

#include <array>
#include <utility>

namespace experiment
{

using std::array;
using std::size_t;

namespace detail
{

template<typename F, typename Head, typename ...Tail>
auto reduce( F f, Head h, Tail... t)
{
  return f(h, reduce(f,t...));
}

template<typename F, typename Lhs, typename Rhs>
auto reduce( F f, Lhs lhs, Rhs rhs)
{
  return f(lhs, rhs);
}

template<typename F, typename T, size_t N>
T reduce( F f, const array<T,N> & v )
{
  T result = v[0];
  for (size_t i = 1; i < N; ++i)
  {
    result = f(result, v[i]);
  }
  return result;
}

template<typename F>
struct reducer
{
  F f;

public:
  reducer( F f ): f(f) {}

  template <typename ...T>
  auto operator()(T... inputs)
  {
    return detail::reduce(f, inputs...);
  }
};

///

template<size_t N, typename First, typename Second>
auto operator+(array<First,N> first, array<Second,N> second)
{
  typedef decltype( std::declval<First>() + std::declval<Second>() ) Result;

  array<Result, N> output;
  for (size_t i = 0; i < N; ++i)
  {
    output[i] = first[i] + second[i];
  }
  return output;
}

struct sum
{
  template <typename Lhs, typename Rhs>
  auto operator()( const Lhs & lhs, const Rhs & rhs ) -> decltype(lhs + rhs)
  {
    return lhs + rhs;
  }
};

}

namespace proc {

struct sum : detail::reducer<detail::sum>
{
  sum(): detail::reducer<detail::sum>(detail::sum()) {}
};

}

template <typename ...T>
auto sum( T... input )
{
  return detail::reduce( detail::sum(), input... );
}

}

#endif // STREAMS_EXPERIMENTAL_INCLUDED
