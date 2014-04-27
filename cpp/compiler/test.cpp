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
#if 0
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

    node loop( new scalar_op("*", 2) );
    loop.iterations() = { 3, 4 };

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


    // Create kernel

    kernel k("krnl", &loop, context, devices);

    if (k.error())
    {
        cout << "Failed to create kernel." << endl;
        return 1;
    }
    else
        cout << "Kernel created OK." << endl;

    // Allocate IO

    vector<float*> in_mem( loop.input_count() );
    vector<float*> out_mem( loop.output_count() );

    vector<cl::Buffer> in_buf( loop.input_count() );
    vector<cl::Buffer> out_buf( loop.output_count() );

    for (int i = 0; i < loop.input_count(); ++i)
    {
        cout << "Input size = " << loop.input_size(i).area() << endl;
        size_t bytes = loop.input_size(i).area() * sizeof(float);
        in_mem[i] = (float*) _aligned_malloc(bytes, 16);
        in_buf[i] = cl::Buffer(context, CL_MEM_USE_HOST_PTR, bytes, in_mem[i], &err);
        if (!check_cl_error(err, "Could not create input buffer."))
            return 1;
    }
    for (int i = 0; i < loop.output_count(); ++i)
    {
        cout << "Output size = " << loop.output_size(i).area() << endl;
        size_t bytes = loop.output_size(i).area() * sizeof(float);
        out_mem[i] = (float*) _aligned_malloc(bytes, 16);
        out_buf[i] = cl::Buffer(context, CL_MEM_USE_HOST_PTR, bytes, out_mem[i], &err);
        if (!check_cl_error(err, "Could not create input buffer."))
            return 1;
    }

    if (!k.set_data(in_buf, out_buf))
        return 1;
    else
        cout << "Data preparation OK." << endl;

    // Create OpenCL queue

    cl::CommandQueue cmd_queue(context, devices[0], 0, &err);
    if (!check_cl_error(err, "Failed to create command queue."))
      return 1;

    // Run
#if 1
    bool ok = k.run(cmd_queue);
    if (!ok)
        cerr << "Failed to queue kernel." << endl;
    else
        cout << "Kernel enqueued OK." << endl;

    err = cmd_queue.finish();
    if (!check_cl_error(err, "Problem running kernel."))
        return 1;
    else
        cout << "Kernel run OK." << endl;
#endif
    for (float *mem : in_mem)
        _aligned_free(mem);
    for (float *mem : out_mem)
        _aligned_free(mem);

    return 0;
}
