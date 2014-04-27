#include "util.hpp"
#include "graph.hpp"
#include "kernel.hpp"
#include "code.hpp"
#include "functions.hpp"


#include <iostream>

using namespace stream_graph;
using namespace stream_util;
using namespace std;

int main()
{
#if 1
    composite_function f;

    node *a = new node( new scalar_op("*", 2) );
    a->iterations() = { 5 };

    f.children.push_back( a );
    //f.children.push_back( new node( new scalar_op("*", 2) ) );
    //f.children.push_back( new node( new scalar_op("+", 3) ) );

    //node loop( new scalar_op("*", 2) );
    node loop( &f );
    loop.input_rates() = { {5} };
    loop.iterations() = { 10, 2 };
#endif
#if 0
    node loop( new scalar_op("*", 2) );
    loop.iterations() = { 3, 4 };
#endif

    try
    {
        coordinator c(&f);

        c.run();
    }
    catch (std::exception & e)
    {
        cerr << "ERROR: " << e.what() << endl;
    }

    return 0;
}
