#ifndef STREAMS_PROCESSORS_INCLUDED
#define STREAMS_PROCESSORS_INCLUDED

#include "meta.hpp"

namespace streams {

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

namespace detail {

struct basic_sum
{
  template <typename Lhs, typename Rhs>
  auto operator()( const Lhs & lhs, const Rhs & rhs ) -> decltype(lhs + rhs)
  {
    return lhs + rhs;
  }
};

struct basic_square
{
  template <typename T>
  T operator()(const T & in) { return in * in; }
};

}

struct sum : detail::reducer<detail::basic_sum>
{
  sum(): detail::reducer<detail::basic_sum>(detail::basic_sum()) {}
};

struct square : detail::mapper<detail::basic_square>
{
  square(): detail::mapper<detail::basic_square>(detail::basic_square()) {}
};


} // namespace streams

#endif // STREAMS_PROCESSORS_INCLUDED
