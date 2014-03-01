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

template<size_t I> struct ElementIndex { static const size_t value = I; };

#if 1
template <typename Elements, typename Input, typename Index>
struct chainer
{
  static auto output( Elements & elems, Input & input )
  {
    //return 5.0;
         auto intermediate = chainer<Elements, Input, ElementIndex<Index::value+1>>::output(elems, input);
    return std::get<Index::value>(elems)(intermediate);
  }
};

template <typename Elements, typename Input>
struct chainer<Elements, Input, ElementIndex<std::tuple_size<Elements>::value-1> >
{
  //const size_t a = std::tuple_size<Elements>::value;
  //static float output( Elements & elems, stream<Input> & input ) { return 1.0; }

  static auto output( Elements & elems, Input & input )
  {
    return std::get< std::tuple_size<Elements>::value-1 >(elems)( input );
  }
};
#endif

template <typename ...ElementTypes>
struct series
{
  tuple<ElementTypes...> elements;

  series( ElementTypes... e ): elements(e...) {}

  template <typename Input>
  auto operator()( Input input )
  {
    //test(elements);
    //return 1.0;
    return chainer<tuple<ElementTypes...>, Input, ElementIndex<0>>::output(elements, input);
    //return chain<0,InputType>(input);
  }
};

template <typename ...ElementTypes>
series<ElementTypes...> make_series(  ElementTypes... e )
{
  return series<ElementTypes...>(e...);
}

template <typename ...ElementTypes>
struct parallel
{
  tuple<ElementTypes...> elements;

  parallel( ElementTypes... e ): elements(e...) {}

  template <typename ... Inputs>
  auto operator()( tuple<Inputs...> inputs )
  {
    return outputs_for< sizeof...(Inputs) - 1, Inputs... >::get(elements, inputs );
  }

private:
  template <size_t I, typename ...Inputs>
  struct outputs_for
  {
    static auto get( tuple<ElementTypes...> elements, tuple<Inputs...> & inputs )
    {
      auto output = std::get<I>(elements)( std::get<I>(inputs) );
      return tuple_cat( outputs_for<I-1, Inputs...>::get(elements, inputs), make_tuple(output) );
    }
  };

  template <typename ...Inputs>
  struct outputs_for<0, Inputs...>
  {
    static auto get( tuple<ElementTypes...> elements, tuple<Inputs...> & inputs )
    {
      auto output = std::get<0>(elements)( std::get<0>(inputs) );
      return make_tuple(output);
    }
  };
};

template <typename ...Elements>
parallel<Elements...> make_parallel(  Elements... e )
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

