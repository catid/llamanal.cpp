#ifndef ORACLE_HPP
#define ORACLE_HPP

#include <string>

namespace analysis {


//------------------------------------------------------------------------------
// Oracle

/*
    Object that contains Large Language Model code, specialized for getting back
    a value from 0..1 to rate something.
*/
class Oracle
{
public:
    ~Oracle()
    {
        Shutdown();
    }

    bool Initialize();
    void Shutdown();

    bool QueryRating(std::string prompt, float& rating);
};


} // namespace analysis

#endif // ORACLE_HPP
