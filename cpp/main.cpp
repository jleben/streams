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

  cout << "< (3*2)^2, (2^2)*3 > = " << print(n()) << endl;

  //auto s = n();

  auto noiz = parallelize( accumulate<2>( noise() ),
                           accumulate<3>( noise() ) );

  cout << "parallel(accumulate(noise)) = " << print(noiz()) << endl;

  auto noiz2 = serialize (noise(),
                          fork<2>(),
                          parallelize( square(),
                                       square() ) );
  cout << "noise fork parallel(square) = " << print(noiz2()) << endl;
  //auto s2 = n2();

  auto n3 = serialize( accumulate<2>( make_constant(2) ),
                       split(),
                       fork<2>(),
                       join() );

  cout << "split, fork, join: " << print(n3()) << endl;

#endif


  auto a = serialize( accumulate<2>( make_constant(2) ),
                      square(),
                      sum() );

  auto b = serialize( accumulate<2>( make_constant( make_array(1,2,3)) ),
                      printer(),
                      square(),
                      sum() );

  auto x = serialize( square(), fork<2>(), sum() );

  cout << "a = " << print(a()) << endl;
  cout << "b = " << print(b()) << endl;

  cout << "x(2) = " << print( x(2) ) << endl;
  auto input = make_array(make_array(1,2),make_array(3,4));
  cout << "x(" << print(input) << ") = " << print( x( input ) ) << endl;

  return 0;
}
