#include "dataflow.hpp"

#include <iostream>
#include <array>

using namespace std;
using namespace streams;

int main()
{
  auto n = serialize (
        streams::generate<float>(),
        collect<3>(),
        collect<2>(),
        map(),
        split(),
        parallelize( reduce(), map() ),
        parallelize( map(), reduce() ) );

  //auto s = n();

  auto n2 = parallelize( serialize(
                           streams::generate<int>(),
                           collect<2>()),
                         serialize(
                           streams::generate<float>(),
                           collect<3>()) );
  //auto s2 = n2();

  //print_stream_type(s2);

  auto n3 = serialize( accumulate<2>( accumulate<3>( constant<int>(2) ) ),
                       split(),
                       printer() );
  //auto v3 = n3.process();

  auto x = accumulate<1000>(
        serialize( constant<double>(1/10.0), sine() ) );

  print(x.process());

  return 0;
}
