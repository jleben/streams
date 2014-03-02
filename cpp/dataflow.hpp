#include <array>
#include <tuple>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <initializer_list>
#include <utility>

using namespace std;

/////////////// Utilities //////////////

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

/////////////// Flow Manipulators //////////////

template <typename T>
struct stream
{
  typedef T type;
};

template<size_t N>
struct collect
{
  template <typename T>
  stream< array<T,N> > operator()( stream<T> ) { return stream< array<T,N> >(); }
};

struct map
{
  template <typename T>
  stream<T> operator()( stream<T> ) { return stream<T>(); }
};

struct reduce
{
  template <typename T, size_t N>
  stream<T> operator()( stream< array<T,N> > ) { return stream<T>(); }
};

struct split
{
  template <typename T, size_t N>
  auto operator()( stream< array<T,N> > )
  {
    typename lace<N,stream<T>>::type output;
    return output;
  }
};

/////////////// Composites //////////////

template <typename ...Elements>
struct series
{
  tuple<Elements...> elements;

  series( Elements... e ): elements(e...) {}

  template <typename Input>
  auto operator()( Input input )
  {
    return get_output<sizeof...(Elements)-1, Input>::from(elements, input);
  }

private:
  template <size_t I, typename Input>
  struct get_output
  {
    static auto from( tuple<Elements...> & elements, Input & input )
    {
      auto intermediate = get_output<I-1, Input>::from(elements, input);
      return std::get<I>(elements)(intermediate);
    }
  };

  template <typename Input>
  struct get_output<0, Input>
  {
    static auto from( tuple<Elements...> & elements, Input & input )
    {
      return std::get<0>(elements)(input);
    }
  };
};

template <typename ...Elements>
series<Elements...> serialize(  Elements... e )
{
  return series<Elements...>(e...);
}

template <typename ...Elements>
struct parallel
{
  tuple<Elements...> elements;

  parallel( Elements... e ): elements(e...) {}

  template <typename ... Inputs>
  auto operator()( tuple<Inputs...> inputs )
  {
    return get_output< sizeof...(Inputs) - 1, Inputs... >::from(elements, inputs );
  }

private:
  template <size_t I, typename ...Inputs>
  struct get_output
  {
    static auto from( tuple<Elements...> elements, tuple<Inputs...> & inputs )
    {
      auto output = std::get<I>(elements)( std::get<I>(inputs) );
      return tuple_cat( get_output<I-1, Inputs...>::from(elements, inputs), make_tuple(output) );
    }
  };

  template <typename ...Inputs>
  struct get_output<0, Inputs...>
  {
    static auto from( tuple<Elements...> elements, tuple<Inputs...> & inputs )
    {
      auto output = std::get<0>(elements)( std::get<0>(inputs) );
      return make_tuple(output);
    }
  };
};

template <typename ...Elements>
parallel<Elements...> parallelize(  Elements... e )
{
  return parallel<Elements...>(e...);
}

/////////////// Print Flow Types //////////////

template <typename T>
struct type_printer
{
  static void print() { cout << "x"; }
};

template <typename T, size_t N>
struct type_printer< array<T,N> >
{
  static void print()
  {
    cout << "array ( " << N << ", ";
    type_printer<T>::print();
    cout << " )";
  }
};

template <typename T>
struct type_printer< stream<T> >
{
  static void print()
  {
    cout << "stream ( ";
    type_printer<T>::print();
    cout << " )";
  }
};

template <typename Head, typename ...Tail>
struct tuple_printer
{
  static void print()
  {
    type_printer<Head>::print();
    cout << " , ";
    tuple_printer<Tail...>::print();
  }
};

template <typename Tail>
struct tuple_printer<Tail> : type_printer<Tail>
{};

template <typename ...T>
struct type_printer< tuple<T...> >
{
  static void print()
  {
    cout << "tuple ( ";
    tuple_printer<T...>::print();
    cout << " )";
  }
};

template <typename T>
void print_stream_type( T & s )
{
  type_printer<T>::print();
  cout << endl;
}

