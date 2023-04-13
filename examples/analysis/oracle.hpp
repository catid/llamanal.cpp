#ifndef ORACLE_HPP
#define ORACLE_HPP

#include <string>

// ggml headers
#include "llama.h"

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

    bool Initialize(const std::string& model_path);
    void Shutdown();

    bool QueryRating(std::string prompt, float& rating);

protected:
    llama_context* Context = nullptr;
};


} // namespace analysis

#endif // ORACLE_HPP
