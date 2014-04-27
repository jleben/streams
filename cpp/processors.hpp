#ifndef STREAMS_PROCESSORS_INCLUDED
#define STREAMS_PROCESSORS_INCLUDED

#include "meta.hpp"

#include <array>

namespace streams {

using std::array;

template <typename T>
struct constant
{
  T m_value;
  constant(const T & value): m_value(value) {};
  T operator()() { return m_value; }
};

template <typename T>
constant<T> make_constant( const T & v ) { return constant<T>(v); }

template <typename T, typename ...TT>
constant< array<T, sizeof...(TT)+1> > make_constant( const T & v, const TT ... vv )
{
  return array<T, sizeof...(TT)+1>({v, vv...});
}

template <typename C>
struct iterator
{
    int index;
    C container;
    iterator(const C & container, int start = 0):
        index(start), container(container)
    {}
    typename C::value_type operator()() { return container[index++]; }
};

template <typename C>
iterator<C> iterate(const C & container, int start = 0)
{
    return iterator<C>(container, start);
}

//

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

struct noise
{
  int operator()() { return std::rand(); }
};

struct sine
{
  unsigned int m_phase;

  sine(): m_phase(0) {}

  float operator()( float frequency )
  {
    double real_phase = (double) m_phase / std::numeric_limits<unsigned int>::max();
    double output = std::sin(real_phase * 2 * 3.14);

    unsigned int step = std::numeric_limits<unsigned int>::max() * frequency;
    m_phase += step;

    return output;
  }
};

} // namespace streams

#endif // STREAMS_PROCESSORS_INCLUDED
