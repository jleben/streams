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
  T process(const array<T,N> & input)
  {
    return f.process(input);
  }

  template <typename T, size_t N, size_t NN>
  array<T,N> process( const array< array<T,N>, NN > & input )
  {
    array<T,N> output;
    for (int i = 0; i < N; ++i)
    {
      array<T,NN> sub_input;
      for (int ii = 0; ii < NN; ++ii)
      {
        sub_input[ii] = input[ii][i];
      }
      output[i] = process(sub_input);
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
  T process(const T & input)
  {
    return f.process(input);
  }

  template <typename T, size_t N>
  array<T,N> process(const array<T,N> & input)
  {
    array<T,N> output;
    for (int i = 0; i < N; ++i)
    {
      output[i] = process(input[i]);
    }
    return output;
  }
};

} // namespace streams_meta
