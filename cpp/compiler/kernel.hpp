#ifndef STREAM_KERNEL_INCLUDED
#define STREAM_KERNEL_INCLUDED

#include "util.hpp"
#include "graph.hpp"

#include <string>
#include <CL/cl.hpp>

namespace stream_graph {

using std::string;
using stream_util::extent;

class kernel
{
public:
    static std::string code( const string & name, node * );

    kernel( const string & name,
            node *node,
            cl::Context & context,
            std::vector<cl::Device> & devices );

    bool error()
    {
        return m_cl_status != CL_SUCCESS;
    }

    bool set_data( std::vector<cl::Buffer> inputs,
                   std::vector<cl::Buffer> outputs );

    bool run( std::vector<extent> & input_offsets,
              std::vector<extent> & outputs_offsets,
              cl::CommandQueue & cmd_queue );

    bool run( cl::CommandQueue & cmd_queue );

private:
    string m_name;
    node *m_node;
    cl_int m_cl_status;
    cl::Program m_program;
    cl::Kernel m_kernel;
    //std::vector<cl::Buffer> m_inputs;
    //std::vector<cl::Buffer> m_outputs;
};

} // namespace

#endif // STREAM_KERNEL_INCLUDED
