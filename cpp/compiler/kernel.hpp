#ifndef STREAM_KERNEL_INCLUDED
#define STREAM_KERNEL_INCLUDED

#include "graph.hpp"

#include <string>
#include <CL/cl.hpp>

namespace stream_graph {

using std::string;

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

    bool run( );

private:
    string m_name;
    node *m_node;
    cl_int m_cl_status;
    cl::Program m_program;
};

} // namespace

#endif // STREAM_KERNEL_INCLUDED
