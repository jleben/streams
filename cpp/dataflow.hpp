#include <array>
#include <tuple>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <initializer_list>
#include <utility>

#include "meta.hpp"

using namespace std;

namespace streams {

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

//

struct no_input {};
struct no_output {};

template<typename proc, typename input>
struct output_of
{
  typedef decltype( declval<proc>()( declval<input>() ) ) type;
};

template<typename proc>
struct output_of<proc, no_input>
{
  typedef decltype( declval<proc>()() ) type;
};

/////////////// Flow Manipulators //////////////

struct split
{
  template <typename T, size_t N>
  auto operator()( const array<T,N> & input )
  -> decltype( array_to_tuple< N-1, array<T,N> >::value(input) )
  {
    return array_to_tuple< N-1, array<T,N> >::value(input);
  }
};

/////////////// Workers ///////////////

struct square_impl
{
  template <typename T>
  T operator()(const T & in) { return in * in; }
};

struct square : map<square_impl>
{
  square(): map(square_impl()) {}
};

struct sum_impl
{
  template <typename T, size_t N>
  T operator()(const array<T,N> & in)
  {
    return std::accumulate(in.begin(), in.end(), 0);
  }
};

struct sum : reduce<sum_impl>
{
  sum(): reduce(sum_impl()) {}
};

struct noise
{
  int operator()() { return std::rand(); }
};

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

template <typename T, typename ...TT>
array<T, sizeof...(TT)+1> make_array( const T & v, const TT ... vv )
{
  return array<T, sizeof...(TT)+1>({v, vv...});
}

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

/////////////// Composites //////////////

template <typename ...Elements>
struct series
{
  tuple<Elements...> elements;

  series( Elements... e ): elements(e...) {}

  auto operator()()
  {
    return processor< sizeof...(Elements)-1, tuple<Elements...>, no_input >::process(elements, no_input());
  }

  template <typename Input>
  auto operator()( const Input & input )
  {
    return processor< sizeof...(Elements)-1, tuple<Elements...>, Input >::process(elements, input);
  }

private:
  template <size_t I, typename Elems, typename Input>
  struct processor
  {
    static auto process( Elems & elements, const Input & input )
    {
      auto temp = processor<I-1, Elems, Input>::process( elements, input );
      return std::get<I>(elements)(temp);
    }
  };

  template <typename Elems, typename Input>
  struct processor<0, Elems, Input>
  {
    static auto process( Elems & elements, const Input & input )
    {
      return std::get<0>(elements)( input );
    }
  };

  template <typename Elems>
  struct processor<0, Elems, no_input>
  {
    static auto process( Elems & elements, no_input )
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
};

template <typename ...Elements>
parallel<Elements...> parallelize(  Elements... e )
{
  return parallel<Elements...>(e...);
}

template <size_t N, typename Element>
struct accumulator
{
  Element m_element;

  accumulator(const Element & elem): m_element(elem) {}

  array<typename output_of<Element, no_input>::type, N> operator()()
  {
    array<typename output_of<Element, no_input>::type, N> output;
    for (size_t i = 0; i < N; ++i)
    {
      output[i] = m_element();
    }
    return output;
  }
};

template <size_t N, typename Element>
accumulator<N,Element> accumulate( const Element & e )
{
  return accumulator<N,Element>(e);
}

template <typename Element>
struct shredder
{
  Element m_element;

  shredder(const Element & elem): m_element(elem) {}

  template <typename T, size_t N>
  void operator()( const array<T,N> & input )
  {
    for (size_t i = 0; i < N; ++i)
    {
      m_element( input[i] );
    }
  }
};

template <typename Element>
shredder<Element> shred( const Element & e )
{
  return shredder<Element>(e);
}

/////////////// Printer //////////////

namespace printing
{

template <typename T>
struct printer_for { static void print( const T & v ) { cout << v; } };

template <>
struct printer_for<void> { static void print() { cout << "-"; } };

template <typename T, size_t N>
struct printer_for<array<T,N>>
{
  static void print( const array<T,N> & a )
  {
    cout << "[ ";
    for ( size_t i = 0; i < N; ++i )
    {
      if (i > 0)
        cout << ", ";
      printer_for<T>::print(a[i]);
    }
    cout << " ]";
  }
};

template<size_t I, typename Tuple>
struct tuple_printer_for
{
  static void print( const Tuple & t )
  {
    tuple_printer_for<I-1,Tuple>::print(t);
    cout << ", ";
    printer_for<typename tuple_element<I,Tuple>::type>::print( std::get<I>(t) );
  }
};

template<typename Tuple>
struct tuple_printer_for<0,Tuple>
{
  static void print( const Tuple & t )
  {
    printer_for<typename tuple_element<0,Tuple>::type>::print( std::get<0>(t) );
  }
};

template <typename ...T>
struct printer_for<tuple<T...>>
{
  static void print( const tuple<T...> & t )
  {
    cout << "< ";
    tuple_printer_for< sizeof...(T)-1, tuple<T...> >::print(t);
    cout << " >";
  }
};

}

struct printer
{
  void process()
  {
    printing::printer_for<void>::print();
  }

  template <typename T>
  const T & process( const T & input )
  {
    cout << "printer: ";
    printing::printer_for<T>::print(input);
    cout << endl;

    return input;
  }
};

template<typename T>
void print( const T & value )
{
  printing::printer_for<T>::print(value);
  cout << endl;
}

/////////////// Print Flow Types //////////////

template <typename T>
struct type_printer
{
  static void print() { cout << "x"; }
};

template <>
struct type_printer<void>
{
  static void print() { cout << "-"; }
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
void print_stream_type( const T & s )
{
  type_printer<T>::print();
  cout << endl;
}

}
