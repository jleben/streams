#include "compiler/graph.hpp"
#include "compiler/kernel.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <cassert>

using namespace std;
using namespace stream_graph;

namespace stream_graph {

#if 0
struct function_base
{
    vector<extent> m_inputs;
    vector<extent> m_outputs;
    function_base( const vector<extent> & inputs,
                   const vector<extent> & outputs ):
        m_inputs(inputs),
        m_outputs(outputs)
    {}

    int input_count() { return m_inputs.size(); }
    int output_count()  { return m_outputs.size(); }
    extent input_size( int index ) { return m_inputs[index].size(); }
    extent output_size( int index )  { return m_outputs[index].size(); }
};
#endif

#if 0
class binary_op : public function
{
    string m_symbol;

public:
    binary_op( const string & symbol ):
        m_symbol(symbol)
    {}

    int input_count() { return 2; }
    int output_count()  { return 1; }
    extent input_size( int index ) { return extent(1,1); }
    extent output_size( int index ) { return extent(1,1); }

    stream_code::values generate( const stream_code::values & inputs,
                                  stream_code::context & )
    {
        assert(inputs.size() == 2);

        ostringstream code;
        code << inputs[0].id << ' ' << m_symbol << ' ' << inputs[1].id;

        return { stream_code::value(code.str()) };
    }
};
#endif

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
            ctx << *out << " = " << code.str() << endl;
        }
        else
        {
            out.id = code.str();
        }
    }
};

} // namespace

int main()
{

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

    kernel k("krnl", &loop);

    cout << k.code();

#if 0
    stream_code::context ctx( cout );

    stream_code::values in = loop.create_input_values({"x"});
    stream_code::values out = loop.create_output_values({"y"});


    cout << "in = " << in[0].size << endl;
    cout << "out = " << out[0].size << endl;

    loop.generate(in, out, ctx);
    //stream_code::values r = f.generate({in}, ctx);

    //cout << "result = " << r[0].id << endl;
#endif

#if 0
    node a, b, c;
    a.set_function( new constant(2.0) );
    b.set_function( new constant(3.0) );
    c.set_function( new constant(4.0) );

    node mult;
    mult.set_function( new binary_op("*") );
    mult.add(&a);
    mult.add(&b);

    node sum;
    sum.set_function( new binary_op("+") );
    sum.add(&mult);
    sum.add(&c);

    kernel k(&sum);

    k.write(cout);
#endif
}
