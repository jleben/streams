#ifndef STREAM_KERNEL_INCLUDED
#define STREAM_KERNEL_INCLUDED

#include "util.hpp"
#include "graph.hpp"

#include <vector>
#include <list>
#include <string>
#include <CL/cl.hpp>

namespace stream_graph {

using std::vector;
using std::string;
using std::list;
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

class coordinator
{

public:
    coordinator( composite_function * func );

    void run();

private:
    struct kernel_data
    {
        kernel_data(cl::Context &, std::vector<cl::Device> &,
                     node *def, const std::string &name,
                     vector<cl::Buffer> & inputs );
        ~kernel_data();
        node *def;
        kernel krnl;
        vector<float*> in_mem;
        vector<float*> out_mem;
        vector<cl::Buffer> in_buf;
        vector<cl::Buffer> out_buf;
    };

    composite_function *m_func;
    cl::Platform m_platform;
    std::vector<cl::Device> m_devices;
    cl::Context m_context;
    cl::CommandQueue m_cmd_queue;
    list<kernel_data> m_schedule;
};

} // namespace

#endif // STREAM_KERNEL_INCLUDED
