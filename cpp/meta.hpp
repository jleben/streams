#include <array>

namespace streams
{
using std::array;

template <typename F>
class reduce
{
  F f;

public:
  reduce( F f ): f(f) {}

  template <typename T, size_t N>
  T operator()(const array<T,N> & input)
  {
    return f(input);
  }

  template <typename T, size_t N, size_t NN>
  array<T,N> operator()( const array< array<T,N>, NN > & input )
  {
    array<T,N> output;
    for (int i = 0; i < N; ++i)
    {
      array<T,NN> sub_input;
      for (int ii = 0; ii < NN; ++ii)
      {
        sub_input[ii] = input[ii][i];
      }
      output[i] = (*this)(sub_input);
    }
    return output;
  }
};

template <typename F>
class map
{
  F f;

public:
  map( F f ): f(f) {}

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

} // namespace streams_meta
