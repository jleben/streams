#include "flow.hpp"
#include "processors.hpp"
//#include "experiment.hpp"
#include "print.hpp"

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
#endif

  auto noiz = parallelize( accumulate<2>( noise() ),
                           accumulate<3>( noise() ) );

  cout << "parallel(accumulate(noise))"; print(noiz());

  auto noiz2 = serialize (noise(),
                          fork<2>(),
                          parallelize( square(),
                                       square() ) );
  cout << "noise fork parallel(square):"; print(noiz2());
  //auto s2 = n2();

  auto n3 = serialize( accumulate<2>( make_constant(2) ),
                       split(),
                       fork<2>(),
                       join() );

  cout << "split, fork, join: ";
  print(n3());

  //print(x());

  //print( experiment::sum(make_array(1,2), make_array(1,2), make_array(1,2)) );
  //print( experiment::sum(make_array(1,2,3)) );

  //print( serialize( make_constant(make_tuple(1,2,3)), join() )() );


  return 0;
}
