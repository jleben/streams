#ifndef STREAM_FUNCTIONS_INCLUDED
#define STREAM_FUNCTIONS_INCLUDED

#include "model.hpp"

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

class avg_filter : public function
{
    int width;

public:
    avg_filter(int width = 3): width(width)
    { assert(width > 0); }

    int input_count() { return 1; }
    int output_count()  { return 1; }
    extent input_size( int index ) { return {1,width}; }
    extent output_size( int index ) { return {1}; }

    void generate( const stream_code::values & inputs,
                   stream_code::values & outputs,
                   stream_code::context & ctx )
    {
        using stream_code::endl;

        assert(inputs.size() == 1);
        assert(outputs.size() == 1);

        string index = inputs[0].index[1];
        stream_code::values local_inputs = inputs;
        ostringstream code;

        code << "(";

        code << *local_inputs[0];

        for (int i = 1; i < width; ++i)
        {
            ostringstream next_index;
            next_index << index << " + " << (i+1);
            local_inputs[0].index[1] = next_index.str();

            code << " + " << *local_inputs[0];
        }

        code << ") / " << width;

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
