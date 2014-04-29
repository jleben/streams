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
struct printer
{
  const T & value;
  printer( const T & v ) : value(v) {}
};

template <>
struct printer<void>
{};

template <typename T>
printer<T> printer_for(const T & v) { return printer<T>(v); }

// concrete printing functions

template <typename T>
std::ostream & operator<< (std::ostream & s, const printer<T> & p)
{
  s << p.value;
  return s;
}

std::ostream & operator<< (std::ostream & s, const printer<void> & p)
{
  s << "-";
  return s;
}

// array printing

template <typename T, size_t N>
std::ostream & operator<< (std::ostream & s, const printer< array<T,N> > & p)
{
  s << "[ ";
  for ( size_t i = 0; i < N; ++i )
  {
    if (i > 0)
      s << ", ";
    s << printer_for(p.value[i]);
  }
  s << " ]";
  return s;
}

// tuple printing

template<size_t I> struct tuple_index {};

template <typename Tuple, size_t I>
void print_tuple(std::ostream & s, const Tuple & t, tuple_index<I>)
{
  print_tuple(s, t, tuple_index<I-1>());
  s << ", ";
  s << printer_for(std::get<I-1>(t));
}

template <typename Tuple>
void print_tuple(std::ostream & s, const Tuple & t, tuple_index<1>)
{
  s << printer_for(std::get<0>(t));
}

template <typename ...T>
std::ostream & operator<< (std::ostream & s, const printer<tuple<T...> > & p)
{
  s << "< ";
  print_tuple(s, p.value, tuple_index<tuple_size<tuple<T...> >::value>());
  s << " >";
}

}

struct printer
{
  void operator()()
  {
    cout << "printer: " << printing::printer<void>() << endl;
  }

  template <typename T>
  const T & operator()( const T & input )
  {
    cout << "printer: " << printing::printer_for(input) << endl;
    return input;
  }

  template <typename ...T>
  auto operator()( const T & ... inputs ) -> decltype(make_tuple(inputs...))
  {
    auto t = make_tuple(inputs...);
    cout << "printer: " << printing::printer_for(t) << endl;
    return t;
  }
};

template<typename T>
auto print( const T & value ) -> decltype(printing::printer_for(value))
{
  return printing::printer_for(value);
}


/////////////// Print Flow Types //////////////

#if 0
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
#endif

} // namespace streams

#endif // STREAMS_PRINTING_INCLUDED
