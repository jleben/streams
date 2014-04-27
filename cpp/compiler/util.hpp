#ifndef STREAM_UTIL_INCLUDED
#define STREAM_UTIL_INCLUDED

#include <initializer_list>
#include <vector>
#include <algorithm>
#include <iostream>
#include <CL/cl.hpp>

namespace stream_util
{

struct extent : public std::vector<int>
{
    extent()
    {}

    extent( std::initializer_list<int> l ):
        std::vector<int>(l)
    {}

    explicit extent( int count, int value = 1 ):
        std::vector<int>(count, value)
    {}

    extent( const std::vector<int> & v ):
        std::vector<int>(v)
    {}

    int at( int dimension ) const
    {
        if (dimension < size())
            return std::vector<int>::at(dimension);
        else
            return 1;
    }

    int count() const { return std::vector<int>::size(); }
};


//typedef std::vector<int> extent;

inline std::ostream & operator<<( std::ostream & s, const extent & size )
{
    s << "< ";
    for (int i : size)
        s << i << " ";
    s << ">";
}

inline bool check_cl_error(cl_int err, const char * what)
{
  if (err != CL_SUCCESS)
  {
    std::cerr << "ERROR (" << err << "): " << what << std::endl;
    return false;
  }
  return true;
}

} // namespace

#endif // STREAM_UTIL_INCLUDED
