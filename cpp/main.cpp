#include "dataflow.hpp"
#include "experiment.hpp"

#include <iostream>
#include <array>

using namespace std;
using namespace streams;

int main()
{
#if 1
  auto n = serialize (
        accumulate<2>( accumulate<3>( make_constant(2) ) ),
        split(),
        parallelize( sum(), square() ),
        parallelize( square(), sum() ) );

  cout << "< (3*2)^2, (2^2)*3 > = ";
  print(n());

  //auto s = n();

  auto n2 = parallelize( serialize(
                           accumulate<2>( noise() ) ),
                         serialize(
                           accumulate<3>( noise() ) ) );
  //auto s2 = n2();

  //print_stream_type(s2);
#endif

  auto n3 = serialize( accumulate<2>( accumulate<3>( constant<int>(2) ) ),
                       split(),
                       printer() );
  //auto v3 = n3.process();

  auto x = serialize( accumulate<2>( make_constant( make_array(1,2,3), make_array(-1,-2,-3) ) ),
                      square() );

  //print(x());

  auto v = experiment::sum(make_array(1,2), make_array(3,4));
  print(v);

  //cout << experiment::sum(make_array(1,2), make_array(3,4)) << endl;

  return 0;
}
