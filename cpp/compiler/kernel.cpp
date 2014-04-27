#include "kernel.hpp"
#include "code.hpp"

#include <sstream>
#include <vector>

using namespace std;

namespace stream_graph {

using namespace stream_code;

string kernel::code()
{
    // TODO: allow values to originate from outside
    // (if larger than this kernel's IO range).

    using stream_code::endl;

    ostringstream out;
    stream_code::context ctx( out );

    // Generate IO names

    vector<string> input_names;
    for (int i = 0; i < m_node->input_count(); ++i)
    {
        ostringstream name;
        name << "input" << i;
        input_names.push_back( name.str() );
    }

    vector<string> output_names;
    for (int i = 0; i < m_node->output_count(); ++i)
    {
        ostringstream name;
        name << "output" << i;
        output_names.push_back( name.str() );
    }

    // Create IO values
    values inputs = m_node->create_input_values(input_names);
    values outputs = m_node->create_input_values(output_names);

    // Generate declaration

    ctx << "__kernel void " << m_name;
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
    for (int d = 0; d < m_node->iterations().count(); ++d)
    {
        ostringstream index;
        index << "g_id_" << d;
        global_indexes.push_back(index.str());

        ctx << "unsigned int " << index.str() << " = get_global_id(" << d << ");" << endl;
    }

    for (int i = 0; i < inputs.size(); ++i)
    {
        node::add_indexes( inputs[i], global_indexes, m_node->input_rates()[i], ctx );
    }

    // Generate function

    m_node->func()->generate(inputs, outputs, ctx);

    // End body

    ctx.unindent();
    ctx << "}" << endl;

    return out.str();
}

}
