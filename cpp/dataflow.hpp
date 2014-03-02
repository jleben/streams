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

/////////////// Streams ///////////////

template <typename T>
struct stream
{
  typedef T type;
};

/////////////// Workers ///////////////

namespace streams {

template <typename T>
struct generate
{
  stream<T> operator()() { return stream<T>(); }
};

}

/////////////// Flow Manipulators //////////////

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

  auto operator()()
  {
    return get_output<sizeof...(Elements)-1, no_input, tuple<Elements...>>::from(elements, no_input());
  }

  template <typename Input>
  auto operator()( Input input )
  {
    return get_output<sizeof...(Elements)-1, Input, tuple<Elements...>>::from(elements, input);
  }

private:
  struct no_input {};

  template <size_t I, typename Input, typename Elem>
  struct get_output
  {
    static auto from( Elem & elements, const Input & input )
    {
      auto intermediate = get_output<I-1, Input, Elem>::from(elements, input);
      return std::get<I>(elements)(intermediate);
    }
  };

  template <typename Input, typename Elem>
  struct get_output<0, Input, Elem>
  {
    static auto from( Elem & elements, const Input & input )
    {
      return std::get<0>(elements)(input);
    }
  };

  template <typename Elem>
  struct get_output<0, no_input, Elem>
  {
    static auto from( Elem & elements, const no_input & input )
    {
      return std::get<0>(elements)();
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

  auto operator()()
  {
    return get_output< sizeof...(Elements)-1, NoInput, tuple<Elements...> >::from(elements, NoInput());
  }

  template <typename ... Inputs>
  auto operator()( tuple<Inputs...> inputs )
  {
    return get_output< sizeof...(Elements)-1, tuple<Inputs...>, tuple<Elements...> >::from(elements, inputs );
  }

private:
  struct NoInput {};
  template<size_t> struct Index {};

  template<typename Elem, typename In, size_t I>
  struct get_elem_output
  {
    static auto from( Elem & elem, const In & inputs, Index<I> )
    {
      return elem( std::get<I>(inputs) );
    }
  };

  template<typename Elem, size_t I>
  struct get_elem_output<Elem, NoInput, I>
  {
    static auto from( Elem & elem, const NoInput &, Index<I> )
    {
      return elem();
    }
  };

  template<typename Elem, typename In, size_t I>
  static auto get_elem_output_from( Elem & elem, const In & inputs, Index<I> index )
  {
    return get_elem_output<Elem,In,I>::from(elem, inputs, index);
  }

  template <size_t I, typename In, typename Elem>
  struct get_output
  {
    static auto from( Elem & elements, const In & inputs )
    {
      auto output = get_elem_output_from(std::get<I>(elements), inputs, Index<I>());
      return tuple_cat( get_output<I-1, In, Elem>::from(elements, inputs), make_tuple(output) );
    }
  };

  template <typename In, typename Elem>
  struct get_output<0, In, Elem>
  {
    static auto from( Elem & elements, const In & inputs )
    {
      auto output = get_elem_output_from(std::get<0>(elements), inputs, Index<0>());
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

