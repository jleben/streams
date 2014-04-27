#ifndef STREAMS_GRAPH_INCLUDED
#define STREAMS_GRAPH_INCLUDED

#include "code.hpp"
#include "util.hpp"

#include <vector>
#include <list>
#include <map>
#include <string>
#include <iostream>
#include <algorithm>

#undef max

namespace stream_graph {

using namespace stream_util;

using stream_util::extent;

struct node;

enum function_type
{
    primitive = 0,
    composite
};

struct function : public stream_code::generator
{
    virtual ~function() {}
    virtual int type() const { return primitive; }

    virtual int input_count() = 0;
    virtual int output_count() = 0;
    virtual extent input_size( int index ) = 0;
    virtual extent output_size( int index ) = 0;
};

struct composite_function : public function
{
    int type() const { return composite; }

    int input_count();
    int output_count();
    extent input_size( int index );
    extent output_size( int index );

    void generate( const stream_code::values & inputs,
                   stream_code::values & outputs,
                   stream_code::context & );

    //stream_code::values generate( const stream_code::values & inputs, stream_code::context & );

    std::list<node*> children;
};

struct node : public stream_code::generator
{
    node (function *func):
        m_parent(nullptr),
        m_func(func)
    {
        for (int i = 0; i < func->input_count(); i++)
        {
            extent input_size = func->input_size(i);
            m_input_rates.emplace_back( input_size.size(), 1 );
        }

        // FIXME:
        m_iterations = {1};
    }

    ~node()
    {}

    bool is_composite() { return m_func->type() == composite; }

    function * func() { return m_func; }

    composite_function * composite_func()
    {
        if (m_func->type() != composite)
            throw std::runtime_error("Not a graph.");
        return reinterpret_cast<composite_function*>(m_func);
    }

    std::vector<extent> & input_rates() { return m_input_rates; }
    extent & iterations() { return m_iterations; }

    int input_count() const { return m_func->input_count(); }
    int output_count() const { return m_func->output_count(); }
    extent input_size(int index);
    extent output_size(int index);

    bool can_merge( node * other, bool downstream );
    void merge( node * other, bool downstream );

    void generate( const stream_code::values & inputs,
                   stream_code::values & outputs,
                   stream_code::context & );

    stream_code::values allocate_input_values()
    {
        return create_input_values( std::vector<std::string>( input_count(), "" ) );
    }

    stream_code::values allocate_output_values()
    {
        return create_input_values( std::vector<std::string>( output_count(), "" ) );
    }


    stream_code::values create_input_values( const std::vector<std::string> & names )
    {
        assert(names.size() == input_count());
        stream_code::values v;
        for (int i =  0; i < input_count(); ++i)
        {
            v.push_back( stream_code::value(names[i], input_size(i)) );
        }
        return v;
    }

    stream_code::values create_output_values( const std::vector<std::string> & names )
    {
        assert(names.size() == output_count());
        stream_code::values v;
        for (int i =  0; i < output_count(); ++i)
        {
            v.push_back( stream_code::value(names[i], output_size(i)) );
        }
        return v;
    }

    static void add_indexes ( stream_code::value & value,
                              const std::vector<std::string> & indexes,
                              const extent & rates,
                              stream_code::context & ctx );

private:
    int total_iteration_size( int hop, int window, int count )
    {
        int hop_count = std::max(0, count - 1);
        return window + hop_count * hop;
    }

    node *m_parent;
    function *m_func;

    std::vector<extent> m_input_rates;
    extent m_iterations;
};

} // namespace


#endif // STREAMS_GRAPH_INCLUDED
