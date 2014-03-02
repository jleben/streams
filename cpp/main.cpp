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

  auto n3 = serialize( constant<array<int,3>>({1,2,3}), split() );
  auto v3 = n3.process();

  //print_stream_type(n3());
  //cout << n3.process() << endl;
  cout << get<0>(v3) << " "
       << get<1>(v3) << " "
       << get<2>(v3) << " "
       << endl;

  output_of< series<constant<array<int,3>>, split>, no_input>::type x;
  cout << std::is_same<decltype(x), tuple<int,int,int>>::value << endl;

  return 0;
}
