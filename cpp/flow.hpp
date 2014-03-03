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

}
