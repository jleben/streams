#include "utility.hpp"
#include "meta.hpp"

#include <array>
#include <tuple>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <initializer_list>
#include <utility>

using namespace std;

namespace streams {

/////////////// IO Introspection //////////////

template<typename proc, typename input>
struct output_of
{
  typedef decltype( declval<proc>()( declval<input>() ) ) type;
};

template<typename proc>
struct output_of<proc, ignore>
{
  typedef decltype( declval<proc>()() ) type;
};

////////////// Process Invokation Helpers /////////////

namespace detail
{

template <size_t I>
struct processor
{
  template <typename P, typename T, typename ...IN>
  static auto process(P &p, const T & t, const IN & ... in)
  {
    return processor<I-1>::process(p, t, get<I-1>(t), in... );
  }
};

template <>
struct processor<0>
{
  template <typename P, typename T, typename ...IN>
  static auto process(P &p, const T & t, const IN & ... in)
  {
    return p ( in... );
  }
};

}

template <typename proc_type, typename ...input_type>
auto process(proc_type & p, const input_type & ... input){ return p(input...); }

template <typename proc_type>
auto process(proc_type & p, const ignore & input){ return p(); }

template <typename proc, typename ...inputs>
auto process(proc & p, const tuple<inputs...> & t)
{
  return detail::processor<sizeof...(inputs)>::process(p, t);
}

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

template<size_t N>
struct fork
{
  template <typename T>
  auto operator()( const T & input ) -> decltype(tuplicate<N>(input))
  {
    return tuplicate<N>(input);
  }

  template <typename ...T>
  auto operator()( const T &... input ) -> decltype(tuplicate<N>(make_tuple(input...)))
  {
    return tuplicate<N>( make_tuple(input...) );
  }
};

struct join
{
  template <typename ...T>
  auto operator()( const T &... input )
  {
    return make_array(input...);
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
    return processor< sizeof...(Elements)-1, tuple<Elements...>, ignore >::process(elements, ignore());
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
      return streams::process(std::get<I>(elements), temp);
    }
  };

  template <typename Elems, typename Input>
  struct processor<0, Elems, Input>
  {
    static auto process( Elems & elements, const Input & input )
    {
      return streams::process(std::get<0>(elements), input);
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
    return no_input_processor<sizeof...(Elements), tuple<Elements...> >::process(elements);
  }

  template <typename ...Input>
  auto operator()(Input & ... input)
  {
    return processor<sizeof...(Elements), tuple<Elements...> >::process(elements, make_tuple(input...));
  }

private:
  template <size_t I, typename Elems>
  struct processor
  {
    template<typename Inputs>
    static auto process(Elems & e, const Inputs & inputs)
    {
      return tuple_cat( processor<I-1, Elems>::process(e,inputs),
                        make_tuple( streams::process(get<I-1>(e), get<I-1>(inputs)) ) );
    }
  };

  template <typename Elems>
  struct processor<1,Elems>
  {
    template<typename Input>
    static auto process(Elems & e, const Input & inputs)
    {
      return make_tuple( streams::process(get<0>(e), get<0>(inputs)) );
    }
  };

  template <size_t I, typename Elems>
  struct no_input_processor
  {
    static auto process(Elems & e)
    {
      return tuple_cat( no_input_processor<I-1,Elems>::process(e),
                        make_tuple( get<I-1>(e)() ) );
    }
  };

  template <typename Elems>
  struct no_input_processor<1,Elems>
  {
    static auto process(Elems & e)
    {
      return make_tuple( get<0>(e)() );
    }
  };
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

  array<typename output_of<Element, ignore>::type, N> operator()()
  {
    array<typename output_of<Element, ignore>::type, N> output;
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
