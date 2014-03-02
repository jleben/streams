#include "dataflow.hpp"

#include <iostream>
#include <array>

using namespace std;

int main()
{

  //auto n = serialize( reduce(), map(), collect<2>(), collect<3>() );
  //auto n = serialize( make_parallel( reduce(), map() ), split(), map(), collect<2>(), collect<3>() );
  auto n = serialize (
        streams::generate<float>(),
        collect<3>(),
        collect<2>(),
        map(),
        split(),
        parallelize( reduce(), map() ),
        parallelize( map(), reduce() ) );
  //auto n = serialize( map(), collect<2>(), collect<3>() );

  auto s = n( );

  auto n2 = parallelize( collect<2>(), collect<3>() );
  auto s2 = n2( make_tuple(stream<float>(), stream<float>()));

  //auto n3 = make_parallel( reduce(), map() );
  //auto s3 = n3(s);


  print_stream_type(s);

  lace<3,float>::type a;
  get<0>(a) = 1;
  get<1>(a) = 2;
  get<2>(a) = 3;

  return 0;
}
