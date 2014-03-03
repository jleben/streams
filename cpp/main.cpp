#include "dataflow.hpp"

#include <iostream>
#include <array>

using namespace std;
using namespace streams;

int main()
{
#if 0
  auto n = serialize (
        accumulate<2>( accumulate<3>( make_constant(3) ) ),
        square(),
        split(),
        parallelize( sum(), square() ),
        parallelize( square(), sum() ) );

  print (n.process());

  //auto s = n();

  auto n2 = parallelize( serialize(
                           streams::generate<int>(),
                           collect<2>()),
                         serialize(
                           streams::generate<float>(),
                           collect<3>()) );
  //auto s2 = n2();

  //print_stream_type(s2);
#endif

  auto n3 = serialize( accumulate<2>( accumulate<3>( constant<int>(2) ) ),
                       split(),
                       printer() );
  //auto v3 = n3.process();

  auto x = serialize( accumulate<2>( make_constant( make_array(1,2,3), make_array(-1,-2,-3) ) ),
                      square() );

  print(x());

  return 0;
}
