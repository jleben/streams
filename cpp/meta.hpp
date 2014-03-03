#ifndef STREAMS_META_INCLUDED
#define STREAMS_META_INCLUDED

#include <array>
#include <utility>

namespace streams
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

template <typename F>
class mapper
{
  F f;

public:
  mapper( F f ): f(f) {}

  template <typename T>
  T operator()(const T & input)
  {
    return f(input);
  }

  template <typename T, size_t N>
  array<T,N> operator()(const array<T,N> & input)
  {
    array<T,N> output;
    for (int i = 0; i < N; ++i)
    {
      output[i] = (*this)(input[i]);
    }
    return output;
  }
};

} // namespace detail

} // namespace streams

#endif // STREAMS_META_INCLUDED
