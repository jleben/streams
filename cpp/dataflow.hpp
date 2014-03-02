#include <array>
#include <tuple>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <initializer_list>
#include <utility>

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
  typedef decltype( declval<proc>().process( declval<input>() ) ) type;
};

template<typename proc>
struct output_of<proc, no_input>
{
  typedef decltype( declval<proc>().process() ) type;
};

/////////////// Streams ///////////////

#if INDEPENDENT_TYPE_LOOKUP
template <typename T>
struct stream
{
  typedef T type;
};
#endif

/////////////// Flow Manipulators //////////////

template<size_t N>
struct collect
{
#if INDEPENDENT_TYPE_LOOKUP
  template <typename T>
  stream< array<T,N> > operator()( stream<T> ) { return stream< array<T,N> >(); }
#endif
};

struct map
{
#if INDEPENDENT_TYPE_LOOKUP
  template <typename T>
  stream<T> operator()( stream<T> ) { return stream<T>(); }
#endif
};

struct reduce
{
#if INDEPENDENT_TYPE_LOOKUP
  template <typename T, size_t N>
  stream<T> operator()( stream< array<T,N> > ) { return stream<T>(); }
#endif
};

struct split
{
#if INDEPENDENT_TYPE_LOOKUP
  template <typename T, size_t N>
  auto operator()( stream< array<T,N> > )
  {
    typename lace<N,stream<T>>::type output;
    return output;
  }
#endif
  template <typename T, size_t N>
  auto process( const array<T,N> & input )
  -> decltype( array_to_tuple< N-1, array<T,N> >::value(input) )
  {
    return array_to_tuple< N-1, array<T,N> >::value(input);
  }
};

/////////////// Workers ///////////////

template <typename T>
struct generate
{
#if INDEPENDENT_TYPE_LOOKUP
  stream<T> operator()() { return stream<T>(); }
#endif
};

struct square : public map
{
  template <typename T>
  T process(const T & in) { return in * in; }
};

struct sum : public reduce
{
  template <typename T, size_t N>
  T process(const array<T,N> & in)
  {
    return std::accumulate(in.begin(), in.end(), 0);
  }
};

struct noise : public generate<int>
{
  int process() { return std::rand(); }
};

template <typename T>
struct constant : public generate<T>
{
  T m_value;
  constant(const T & value): m_value(value) {};
  T process() { return m_value; }
};

struct sine
{
  unsigned int m_phase;

  sine(): m_phase(0) {}

  float process( float frequency )
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

#if INDEPENDENT_TYPE_LOOKUP
  auto operator()()
  {
    return get_output<sizeof...(Elements)-1, no_input, tuple<Elements...>>::from(elements, no_input());
  }

  template <typename Input>
  auto operator()( Input input )
  {
    return get_output<sizeof...(Elements)-1, Input, tuple<Elements...>>::from(elements, input);
  }
#endif
  auto process()
  {
    return processor< sizeof...(Elements)-1, tuple<Elements...>, no_input >::process(elements, no_input());
  }

  template <typename Input>
  auto process( const Input & input )
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
      return std::get<I>(elements).process(temp);
    }
  };

  template <typename Elems, typename Input>
  struct processor<0, Elems, Input>
  {
    static auto process( Elems & elements, const Input & input )
    {
      return std::get<0>(elements).process( input );
    }
  };

  template <typename Elems>
  struct processor<0, Elems, no_input>
  {
    static auto process( Elems & elements, no_input )
    {
      return std::get<0>(elements).process();
    }
  };

#if INDEPENDENT_TYPE_LOOKUP
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
#endif
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

#if INDEPENDENT_TYPE_LOOKUP
  auto operator()()
  {
    return get_output< sizeof...(Elements)-1, NoInput, tuple<Elements...> >::from(elements, NoInput());
  }

  template <typename ... Inputs>
  auto operator()( tuple<Inputs...> inputs )
  {
    return get_output< sizeof...(Elements)-1, tuple<Inputs...>, tuple<Elements...> >::from(elements, inputs );
  }
#endif

private:
#if INDEPENDENT_TYPE_LOOKUP
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
#endif
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

  array<typename output_of<Element, no_input>::type, N> process()
  {
    array<typename output_of<Element, no_input>::type, N> output;
    for (size_t i = 0; i < N; ++i)
    {
      output[i] = m_element.process();
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
  void process( const array<T,N> & input )
  {
    for (size_t i = 0; i < N; ++i)
    {
      m_element.process( input[i] );
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

#if INDEPENDENT_TYPE_LOOKUP
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
#endif

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
