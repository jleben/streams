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

    m_program = cl::Program( context, code, false, &m_cl_status);

    if (!check_cl_error(m_cl_status, "Failed to create program."))
      return;

    m_cl_status = m_program.build(devices, "");
    if (!check_cl_error(m_cl_status, "Failed to build program."))
      return;

    m_kernel = cl::Kernel(m_program, name.c_str(), &m_cl_status);
    if (!check_cl_error(m_cl_status, "Could not create kernel."))
        return;
}

bool kernel::set_data( std::vector<cl::Buffer> inputs,
                       std::vector<cl::Buffer> outputs )
{
    if ( inputs.size() != m_node->input_count() ||
         outputs.size() != m_node->output_count() )
    {
        cerr << "Wrong IO count" << endl;
        return false;
    }

    for (int i = 0; i < inputs.size(); ++i)
    {
        m_cl_status = m_kernel.setArg(i, inputs[i]);
        if (!check_cl_error(m_cl_status, "Failed to set argument."))
            return false;
    }
    for (int i = 0; i < outputs.size(); ++i)
    {
        m_kernel.setArg(inputs.size() + i, outputs[i]);
        if (!check_cl_error(m_cl_status, "Failed to set argument."))
            return false;
    }

    return true;
}

bool kernel::run( std::vector<extent> & input_offsets,
                  std::vector<extent> & outputs_offsets,
                  cl::CommandQueue & cmd_queue )
{
    return true;
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
    return !error();
}

}
