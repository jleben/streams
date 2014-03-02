#include "dataflow.hpp"

#include <iostream>
#include <array>

using namespace std;

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

  auto s = n();

  auto n2 = parallelize( serialize(
                           streams::generate<int>(),
                           collect<2>()),
                         serialize(
                           streams::generate<float>(),
                           collect<3>()) );
  auto s2 = n2();

  print_stream_type(s2);

  return 0;
}
