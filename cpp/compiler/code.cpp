#include "code.hpp"

#include <iostream>

using namespace std;

namespace stream_code {

std::string value::operator*() const
{
    //cout << "defererencing: " << id << std::endl;
    //cout << "index size = " << index.size() << std::endl;
    //cout << "size.size = " << size.size() << std::endl;

    //assert(index.size() <= size.size());

    if (!index.size())
        return id;

    std::stringstream full_index;

    full_index << index.back();

    int period = size.at(index.size() - 1);

    for (int d = index.size() - 2; d >= 0; d--)
    {
        if (index[d].empty())
            continue;
        full_index << " + (" << index[d] << ')';
        if (period != 1)
            full_index << " * " << period;
        period *= size.at(d);
    }

    if (full_index.str().empty())
        return id;

    std::stringstream full_id;
    full_id << id << "[" << full_index.str() << "]";

    return full_id.str();
}

}
