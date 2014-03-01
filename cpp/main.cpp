#include "dataflow.hpp"

#include <iostream>
#include <array>

using namespace std;

int main()
{

  //auto n = make_series( reduce(), map(), collect<2>(), collect<3>() );
  auto n = make_series( split(), map(), collect<2>(), collect<3>() );
  //auto n = make_series( map(), collect<2>(), collect<3>() );

  auto s = n( stream<float>() );

  print_stream_type(s);

  lace<3,float>::type a;
  get<0>(a) = 1;
  get<1>(a) = 2;
  get<2>(a) = 3;

  return 0;
}
