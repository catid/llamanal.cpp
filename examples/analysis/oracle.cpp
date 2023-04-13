#include "oracle.hpp"
#include "logging.hpp"

// ggml headers
#include "common.h"

namespace analysis {


//------------------------------------------------------------------------------
// Oracle

bool Oracle::Initialize(const std::string& model_path)
{
    auto lparams = ::llama_context_default_params();

    lparams.n_ctx      = 2048;
    lparams.n_parts    = 1;
    lparams.seed       = 666;
    lparams.logits_all = false;
    lparams.use_mmap   = true;
    lparams.use_mlock  = false;

    Context = ::llama_init_from_file(model_path.c_str(), lparams);

    return true;
}

void Oracle::Shutdown()
{
    if (Context) {
        ::llama_free(Context);
        Context = nullptr;
    }
}

bool Oracle::QueryRating(std::string prompt, float& rating)
{
    auto tokens = ::llama_tokenize(Context, prompt.c_str(), true);

    return false;
}


} // namespace analysis
