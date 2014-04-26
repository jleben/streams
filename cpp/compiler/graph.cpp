#include "graph.hpp"

#include <stack>
#include <sstream>
#include <limits>
#include <algorithm>
#include <cassert>

using namespace std;

namespace stream_graph {

int composite_function::input_count()
{
    if (children.empty())
        return 0;
    else
        return children.front()->input_count();
}

int composite_function::output_count()
{
    if (children.empty())
        return 0;
    else
        return children.back()->output_count();
}

extent composite_function::input_size( int index )
{
    assert(!children.empty());
    return children.front()->input_size(index);
}

extent composite_function::output_size( int index )
{
    assert(!children.empty());
    return children.back()->output_size(index);
}

////////////////

extent node::input_size(int index)
{
    extent func_size = m_func->input_size(index);

    //assert(size.size() == m_iterations.size() == m_input_rates[index].size());

    int dims = max( func_size.count(), m_iterations.count() );
    extent size(dims);

    for (int d = 0; d < dims; ++d)
    {
        size[d] = total_iteration_size( m_input_rates[index].at(d),
                                        func_size.at(d),
                                        m_iterations.at(d) );
    }

    return size;
}

extent node::output_size(int index)
{
    extent func_size = m_func->output_size(index);

    //assert(size.size() == m_iterations.size());

    int dims = max( func_size.count(), m_iterations.count() );
    extent size(dims);

    for (int d = 0; d < dims; d++)
        size[d] *= m_iterations.at(d);

    return size;
}


bool node::can_merge(node * other_node , bool downstream )
{
    // Iteration counts must be divisible:
#if 0
    if (other_node->m_iterations.size() != m_iterations.size())
    {
        cout << "Iteration dimension mismatch." << endl;
        return false;
    }
#endif
    int dims = max (m_iterations.count(), other_node->m_iterations.count());
    for (int d = 0; d < dims; ++d)
    {
        if (other_node->m_iterations.at(d) % m_iterations.at(d) != 0)
        {
            cout << "Iteration not divisible." << endl;
            return false;
        }
    }

    extent new_iterations(dims);
    for (int d = 0; d < dims; ++d)
        new_iterations[d] = other_node->m_iterations.at(d) / m_iterations.at(d);

    if (downstream)
    {
        assert(other_node->input_count() == output_count());
#if 0
        if (other_node->input_count() != output_count())
        {
            cout << "Input count mismatch." << endl;
            return false;
        }
#endif

        for (int i = 0; i < other_node->input_count(); i++)
        {
            const extent & func_out_size = func()->output_size(i);
            const extent & other_func_in_size = other_node->func()->input_size(i);
            const extent & other_in_rates = other_node->input_rates()[i];

            //assert( func_out_size.size() == other_func_in_size.size() == other_in_rates.size() );

            int dims = max( func_out_size.count(), other_func_in_size.count() );
            dims = max( dims, new_iterations.count() );

            for ( int d = 0; d < dims; d++)
            {
                int new_func_size = total_iteration_size( other_in_rates.at(d),
                                                          other_func_in_size.at(d),
                                                          new_iterations.at(d) );
                if ( new_func_size != func_out_size.at(d) )
                {
                    cout << "Stream format mismatch." << endl;
                    return false;
                }
            }
        }
    }
    else
    {
        assert(other_node->output_count() == input_count());
#if 0
        if (other_node->output_count() != input_count())
        {
            cout << "Output count mismatch." << endl;
            return false;
        }
#endif

        for (int i = 0; i < other_node->output_count(); i++)
        {
            const extent & func_in_size = func()->input_size(i);
            const extent & other_func_out_size = other_node->func()->output_size(i);

            //assert( func_in_size.size() == other_func_out_size.size() );

            int dims = max( func_in_size.count(), other_func_out_size.count() );
            dims = max( dims, new_iterations.count() );

            for ( int d = 0; d < dims; d++)
            {
                int new_func_size = new_iterations.at(d) * other_func_out_size.at(d);
                if (new_func_size != func_in_size.at(d))
                {
                    cout << "Stream format mismatch." << endl;
                    return false;
                }
            }
        }
    }
}

void node::merge(node *other, bool downstream)
{
    composite_function *graph;

    if (!is_composite())
    {
        // convert into a graph, make current function the first child
        graph = new composite_function;
        node *child = new node(m_func);
        graph->children.push_back(child);
        m_func = graph;
    }
    else
    {
        graph = composite_func();
    }

    // convert

    int dims = max (m_iterations.count(), other->m_iterations.count());

    extent new_iterations(dims);
    for (int d = 0; d < dims; ++d)
        new_iterations[d] = other->m_iterations.at(d) / m_iterations.at(d);
    other->m_iterations = new_iterations;


    // move

    // TODO: should be done by whoever is merging,
    // i.e. enclosing composite_function itself

#if 0
    if (other->m_parent)
    {
        assert( other->m_parent->is_composite() );
        composite_function * parent_graph = other->m_parent->composite_func();
        std::remove( parent_graph->children.begin(),
                     parent_graph->children.end(),
                     other );

        // TODO: Confirm that data flow format in parent is correct.
    }
#endif
    other->m_parent = this;

    if (downstream)
    {
        graph->children.push_front(other);
    }
    else
    {
        graph->children.push_back(other);
    }
}

//////////// COMPILATION /////////

void composite_function::generate( const stream_code::values & inputs,
                                   stream_code::values & outputs,
                                   stream_code::context & ctx )
{
    assert( inputs.size() == input_count() );
    assert( outputs.size() == output_count() );

    if (children.size() == 1)
    {
        children.front()->generate( inputs, outputs, ctx );
    }
    else
    {
        stream_code::values tmp = children.front()->allocate_output_values();

        children.front()->generate( inputs, tmp, ctx );

        list<node*>::iterator child_it = ++children.begin();

        while( *child_it != children.back() )
        {
            node *child = *child_it;
            stream_code::values tmp2 = child->allocate_output_values();
            child->generate(tmp, tmp2, ctx);
            tmp = tmp2;
            child_it++;
        }

        children.back()->generate( tmp, outputs, ctx );
    }
}
#if 0
stream_code::values composite_function::generate( const stream_code::values & inputs,
                                              stream_code::context & ctx )
{
    assert( inputs.size() == input_count() );

    stream_code::values values = inputs;

    for (node *child : children)
    {
        values = child->generate( values, ctx );
    }

    assert( values.size() == output_count() );

    return values;
}
#endif

template <typename F>
void for_each( const extent & counts,
               F f,
               stream_code::context & ctx,
               vector<string> & indexes,
               int dimension )
{
    using stream_code::endl;

    if (dimension < counts.size())
    {
        int count = counts[dimension];

        if (count == 0)
            return;

        if (count == 1)
        {
            indexes.push_back(string());
            for_each(counts, f, ctx, indexes, dimension + 1);
            return;
        }

        ctx.push();

        int unique_id = ctx.next_id_index();

        string index;
        {
            ostringstream index_stream;
            index_stream << "d" << unique_id << "_" << dimension;
            index = index_stream.str();
        }
        indexes.push_back(index);

        ctx << "for (" << index << " = 0; "
            << index << " < " << counts[dimension] << "; "
            << index << "++" << ")" << endl;

        ctx << "{" << endl;

        ctx.push();
        ctx.indent();

        for_each(counts, f, ctx, indexes, dimension + 1);

        ctx.unindent();
        ctx.pop();

        ctx << "}" << endl;

        ctx.pop();
    }
    else
    {
        f( indexes );
    }
}

#if 1
template <typename F>
void for_each( const extent & counts,
               F f,
               stream_code::context & ctx )
{
    vector<string> indexes;
    for_each(counts, f, ctx, indexes, 0 );
}
#endif

void node::generate( const stream_code::values & inputs,
                     stream_code::values & outputs,
                     stream_code::context & ctx )
{
    using stream_code::endl;

    stream_code::values indexed_inputs = inputs;
    stream_code::values & indexed_outputs = outputs;

    // FIXME: take iteration rates into account (instead of assuming 1);

    auto work = [&]( const vector<string> & indexes )
    {
        for ( stream_code::value & in : indexed_inputs )
        {
            for (int i = 0; i < indexes.size(); ++i)
            {
                const string & index = indexes[i];
                if (i >= in.index.size())
                    in.index.push_back(string());
                if (!index.size())
                    continue;
                if (in.index[i].size())
                    in.index[i] += " + ";
                in.index[i] += index;
            }
        }
        for ( stream_code::value & out : indexed_outputs )
        {
            for (int i = 0; i < indexes.size(); ++i)
            {
                const string & index = indexes[i];
                if (i >= out.index.size())
                    out.index.push_back(string());
                if (!index.size())
                    continue;
                if (out.index[i].size())
                    out.index[i] += " + ";
                out.index[i] += index;
            }
        }

        m_func->generate(indexed_inputs, indexed_outputs, ctx);
    };

    for_each( m_iterations, work, ctx );
}

#if 0
stream_code::values node::generate( const stream_code::values & inputs,
                                    stream_code::context & ctx )
{
    using stream_code::endl;

    stream_code::values outputs;

    for (int i = 0; i < output_count(); i++)
    {
        ostringstream id;
        id << "v" << ctx.next_id_index();

        ctx << "float " << id.str() << ';' << endl;

        outputs.push_back( stream_code::value(id.str()) );
    }

    auto work = [&]( const vector<string> & indexes )
    {
        stream_code::values func_inputs;

        assert(inputs.size() == m_input_rates.size());

        for (int i = 0; i < inputs.size(); i++)
        {
            ostringstream func_input_id;

            func_input_id << inputs[i].id;

            assert( m_input_rates[i].size() == indexes.size() );

            cout << "indexes: " << indexes.size() << std::endl;
            for (int d = 0; d < indexes.size(); d++)
            {
                int rate = m_input_rates[i][d];
                func_input_id << '[';
                if (rate == 0)
                    func_input_id << 0;
                else if (rate == 1)
                    func_input_id << indexes[d];
                else
                    func_input_id << indexes[d] << " * " << rate;
                func_input_id << ']';
            }

            func_inputs.push_back( stream_code::value(func_input_id.str()) );
            cout << "func in: " << func_inputs.back().id << std::endl;
        }

        stream_code::values func_outputs = m_func->generate(func_inputs, ctx);

        assert( outputs.size() == func_outputs.size() );

        //cout << "..dimension: " << dimension << std::endl;
        //cout << "..indexes = " << indexes.size() << std::endl;

        for (int i = 0; i < func_outputs.size(); i++)
        {
            cout << "func out: " << func_outputs[i].id << std::endl;

            ctx << outputs[i].id;
            for (int d = 0; d < indexes.size(); d++)
            {
                cout << "$";
                // TODO: output ranges???
                ctx << '[' << indexes[d] << ']';
            }
            ctx << " = " << func_outputs[i].id << ';' << endl;
        }
    };

    for_each( m_iterations, work, ctx );

    return outputs;
}
#endif
} // namespace
