#ifndef STREAM_CODE_INCLUDED
#define STREAM_CODE_INCLUDED

#include "util.hpp"

#include <vector>
#include <stack>
#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include <cassert>

namespace stream_code {

using namespace stream_util;

using stream_util::extent;

class context
{
public:
    context( std::ostream & code ):
        m_code(code),
        m_indent(0)
    {
        m_ids.emplace();
    }

    std::string line() { return m_buffer.str(); }

    void push()
    {
        m_ids.push(m_ids.top());
    }

    void pop()
    {
        m_ids.pop();
    }

    void indent() { m_indent++; }
    void unindent() { m_indent--; }

    std::string unique_id( const std::string & id )
    {
        std::ostringstream out;
        out << id;
        out << m_ids.top()[id]++;
        return out.str();
    }

    void endl()
    {
        m_code << std::string(m_indent * 4, ' ') << m_buffer.str() << std::endl;
        m_buffer.str( std::string() );
        m_buffer.clear();
    }

    template <typename T>
    context & operator<< ( const T & object )
    {
        m_buffer << object;
        return *this;
    }

    typedef context & (*manipulator)( context & );

    context & operator<< ( manipulator m )
    {
        return m(*this);
    }

private:
    std::ostream & m_code;
    std::ostringstream m_buffer;
    std::stack< std::map<std::string, int> > m_ids;
    int m_indent;
};

inline
context & endl( context & ctx )
{
    ctx.endl();
    return ctx;
}

struct value
{
    value()
    {}

    value ( const std::string & id ):
        id(id)
    {}

    value ( const std::string & id, const extent & size ):
        id(id),
        size(size)
    {}


    std::string operator*() const;

    std::string id;
    std::vector<std::string> index;
    extent size;
};

typedef std::vector<value> values;

struct generator
{
    virtual void generate( const values & inputs, values & outputs, context & ) = 0;
};

}

#endif // STREAM_CODE_INCLUDED
