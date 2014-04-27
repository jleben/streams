#include "kernel.hpp"
#include "code.hpp"

#include <sstream>
#include <vector>

using namespace std;

namespace stream_graph {

using namespace stream_code;
using namespace stream_util;

string kernel::code( const string & kernel_name, node *node )
{
    // TODO: allow values to originate from outside
    // (if larger than this kernel's IO range).

    using stream_code::endl;

    ostringstream out;
    stream_code::context ctx( out );

    // Generate IO names

    vector<string> input_names;
    for (int i = 0; i < node->input_count(); ++i)
    {
        ostringstream name;
        name << "input" << i;
        input_names.push_back( name.str() );
    }

    vector<string> output_names;
    for (int i = 0; i < node->output_count(); ++i)
    {
        ostringstream name;
        name << "output" << i;
        output_names.push_back( name.str() );
    }

    // Create IO values
    values inputs = node->create_input_values(input_names);
    values outputs = node->create_input_values(output_names);

    // Generate declaration

    ctx << "__kernel void " << kernel_name;
    ctx << "( ";
    bool comma = false;
    for (const string & in : input_names)
    {
        if (comma)
            ctx  << ", ";
        ctx << "__global float * " << in;
        comma = true;
    }
    for (const string & out : output_names)
    {
        if (comma)
            ctx  << ", ";
        ctx << "__global float * " << out;
        comma = true;
    }
    ctx << " )" << endl;

    // Start body

    ctx << "{" << endl;
    ctx.indent();

    // Initialize indexes

    vector<string> global_indexes;
    for (int d = 0; d < node->iterations().count(); ++d)
    {
        ostringstream index;
        index << "g_id_" << d;
        global_indexes.push_back(index.str());

        ctx << "unsigned int " << index.str() << " = get_global_id(" << d << ");" << endl;
    }

    for (int i = 0; i < inputs.size(); ++i)
    {
        node::add_indexes( inputs[i], global_indexes, node->input_rates()[i], ctx );
    }
    for (int i = 0; i < outputs.size(); ++i)
    {
        node::add_indexes( outputs[i], global_indexes, node->func()->output_size(i), ctx );
    }

    // Generate function

    node->func()->generate(inputs, outputs, ctx);

    // End body

    ctx.unindent();
    ctx << "}" << endl;

    return out.str();
}

kernel::kernel( const string & name,
                node *node,
                cl::Context & context,
                vector<cl::Device> & devices ):
    m_name(name),
    m_node(node),
    m_cl_status(CL_SUCCESS)
{
    string code = kernel::code(name, node);

    cout << code;

    m_program = cl::Program( context, code, false, &m_cl_status);

    confirm(m_cl_status, "Failed to create program.");

    m_cl_status = m_program.build(devices, "");
    confirm(m_cl_status, "Failed to build program.");

    m_kernel = cl::Kernel(m_program, name.c_str(), &m_cl_status);
    confirm(m_cl_status, "Could not create kernel.");
}

bool kernel::set_data( std::vector<cl::Buffer> inputs,
                       std::vector<cl::Buffer> outputs )
{
    if ( inputs.size() != m_node->input_count() ||
         outputs.size() != m_node->output_count() )
    {
        throw std::runtime_error("Wrong IO count");
    }

    for (int i = 0; i < inputs.size(); ++i)
    {
        m_cl_status = m_kernel.setArg(i, inputs[i]);
        confirm(m_cl_status, "Failed to set argument.");
    }
    for (int i = 0; i < outputs.size(); ++i)
    {
        m_kernel.setArg(inputs.size() + i, outputs[i]);
        confirm(m_cl_status, "Failed to set argument.");
    }

    return true;
}

bool kernel::run( std::vector<extent> & input_offsets,
                  std::vector<extent> & outputs_offsets,
                  cl::CommandQueue & cmd_queue )
{
    return false;
}

bool kernel::run( cl::CommandQueue & cmd_queue )
{
    extent iter = m_node->iterations();
    cl::NDRange offset;
    // FIXME: use all/only existing dimensions
    cl::NDRange iterations(iter.at(0), iter.at(1), iter.at(2));
    m_cl_status = cmd_queue.enqueueNDRangeKernel(m_kernel,
                                                 offset,
                                                 iterations);
    confirm(m_cl_status, "Failed to run.");
    return true;
}

coordinator::coordinator(composite_function *func):
    m_func(func)
{
    // Init OpenCL:

    cl_int status;

    m_platform = cl::Platform::getDefault(&status);
    confirm(status, "Failed to get default platform.");

    status = m_platform.getDevices(CL_DEVICE_TYPE_ALL, &m_devices);
    confirm(status, "Could not get devices.");

#if 0
    cl::Device device = cl::Device::getDefault(&cl_result);
    if (cl_result != CL_SUCCESS)
    {
      cerr << "Failed to get default device." << endl;
      return 1;
    }
#endif

    m_context = cl::Context(m_devices, nullptr, nullptr, nullptr, &status);
    confirm(status, "Failed to create context.");

    m_cmd_queue = cl::CommandQueue(m_context, m_devices[0], 0, &status);
    confirm(status, "Failed to create command queue.");

    // Build schedule

    std::vector<cl::Buffer> & buffers = std::vector<cl::Buffer>();

    for ( node *def : m_func->children )
    {
        m_schedule.emplace_back( m_context, m_devices, def, "krnl", buffers );
        buffers = m_schedule.back().out_buf;
    }
}

coordinator::kernel_data::kernel_data(cl::Context &context, std::vector<cl::Device> &devices,
                                      node *def, const std::string & name,
                                      vector<cl::Buffer> & inputs):
    def(def),
    krnl(name, def, context, devices)
{
    cl_int status;

    if (inputs.size())
    {
        if (inputs.size() != def->input_count())
            throw std::runtime_error("Invalid nuber of input buffers.");

        in_buf = inputs;
    }
    else
    {
        for (int i = 0; i < def->input_count(); ++i)
        {
            size_t bytes = def->input_size(i).area() * sizeof(float);
            in_mem.push_back( (float*) _aligned_malloc(bytes, 16) );
            in_buf.emplace_back(context, CL_MEM_USE_HOST_PTR, bytes, in_mem.back(), &status);
            confirm(status, "Could not create input buffer.");

            cout << "Input size = " << def->input_size(i).area()
                 << " = " << ( (float) bytes / 1024 / 1024 ) << " Mb" << endl;
        }
    }

    for (int i = 0; i < def->output_count(); ++i)
    {

        size_t bytes = def->output_size(i).area() * sizeof(float);
        out_mem.push_back( (float*) _aligned_malloc(bytes, 16) );
        out_buf.emplace_back(context, CL_MEM_USE_HOST_PTR, bytes, out_mem.back(), &status);
        confirm(status, "Could not create input buffer.");

        cout << "Output size = " << def->output_size(i).area()
             << " = " << ( (float) bytes / 1024 / 1024 ) << " Mb" << endl;
    }

    krnl.set_data(in_buf, out_buf);
    cout << "Data preparation OK." << endl;
}

coordinator::kernel_data::~kernel_data()
{
    for ( float *mem: in_mem )
        _aligned_free(mem);
    for ( float *mem: out_mem )
        _aligned_free(mem);
}

void coordinator::run()
{
    for ( kernel_data & k : m_schedule )
    {
        k.krnl.run(m_cmd_queue);
        cl_int status = m_cmd_queue.finish();
        confirm(status, "Problem running kernel.");
    }
}

}
