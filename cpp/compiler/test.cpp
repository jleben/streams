#include "util.hpp"
#include "graph.hpp"
#include "kernel.hpp"
#include "code.hpp"
#include "functions.hpp"


#include <iostream>
#include <chrono>

using namespace stream_graph;
using namespace stream_util;
using namespace std;
using namespace std::chrono;
typedef std::chrono::high_resolution_clock test_clock;

int main(int argc, char *argv[])
{
    using stream_util::extent;

    int global_iteration_count = (int) 1e6;

    if (argc > 1)
        global_iteration_count = atoi(argv[1]);

    if (global_iteration_count < 1)
    {
        cerr << "Invalid iteration count: " << global_iteration_count << endl;
        return 1;
    }

    cout << "Iteration count: " << global_iteration_count << endl;

    extent iterations = { global_iteration_count, 5, 5 };

    composite_function f;

    node *a = new node( new scalar_op("*", 2) );
    a->iterations() = iterations;

    node *b = new node( new scalar_op("+", 3) );
    b->iterations() = iterations;

    f.children.push_back( a );
    f.children.push_back( b );

    f.optimize();

#if 0
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

        cout << "Running..." << endl;

        test_clock::time_point start = test_clock::now();
        c.run();
        test_clock::time_point end = test_clock::now();

        duration<double, milli> dur = end - start;
        cout << "Duration = " << dur.count() << " ms" << endl;
    }
    catch (std::exception & e)
    {
        cerr << "ERROR: " << e.what() << endl;
    }

    return 0;
}
