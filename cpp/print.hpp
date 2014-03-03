#ifndef STREAMS_PRINTING_INCLUDED
#define STREAMS_PRINTING_INCLUDED

#include <tuple>
#include <array>
#include <iostream>

namespace streams {

using namespace std;

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

} // namespace streams

#endif // STREAMS_PRINTING_INCLUDED
