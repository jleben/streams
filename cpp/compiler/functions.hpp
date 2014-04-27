#ifndef STREAM_FUNCTIONS_INCLUDED
#define STREAM_FUNCTIONS_INCLUDED

#include "graph.hpp"

#include <string>
#include <sstream>

namespace stream_graph {

using std::string;
using std::ostringstream;

class scalar_op : public function
{
    string op;
    float value;

public:
    scalar_op(string op, float value):
        op(op),
        value(value)
    {}

    int input_count() { return 1; }
    int output_count()  { return 1; }
    extent input_size( int index ) { return extent(1,1); }
    extent output_size( int index ) { return extent(1,1); }

    void generate( const stream_code::values & inputs,
                   stream_code::values & outputs,
                   stream_code::context & ctx )
    {
        using stream_code::endl;

        assert(inputs.size() == 1);
        assert(outputs.size() == 1);

        ostringstream code;
        code << *inputs[0] << ' ' << op << ' ' << value;

        stream_code::value & out = outputs[0];
        if ( !out.id.empty() )
        {
            ctx << *out << " = " << code.str() << ";" << endl;
        }
        else
        {
            out.id = code.str();
        }
    }
};

} // namespace

#endif // STREAM_FUNCTIONS_INCLUDED
