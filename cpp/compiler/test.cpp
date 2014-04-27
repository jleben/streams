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

    cout << kernel::code("krnl", &loop);


    // Init OpenCL:

    cl_int err;

    cl::Platform platform = cl::Platform::getDefault(&err);
    if (!check_cl_error(err, "Failed to get default platform."))
      return 1;

    vector<cl::Device> devices;
    err = platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    if (!check_cl_error(err, "Could not get devices."))
      return 1;

#if 0
    cl::Device device = cl::Device::getDefault(&cl_result);
    if (cl_result != CL_SUCCESS)
    {
      cerr << "Failed to get default device." << endl;
      return 1;
    }
#endif
    cl::Context context(devices, nullptr, nullptr, nullptr, &err);
    if (!check_cl_error(err, "Failed to create context."))
      return 1;


    kernel k("krnl", &loop, context, devices);

    cout << "compilation: " << !k.error();

#if 0
    cl::CommandQueue cmd_queue(context, devices[0], 0, &err);
    if (!check_cl_error(err, "Failed to create command queue."))
      return 1;
#endif
}
