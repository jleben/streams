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
    kernel( const string & name, node *node ):
        m_name(name),
        m_node(node)
    {}

    std::string code();

    bool compile( cl::Context &, std::vector<cl::Device> & );

private:
    string m_name;
    node *m_node;
    cl::Program m_program;
};

} // namespace

#endif // STREAM_KERNEL_INCLUDED
