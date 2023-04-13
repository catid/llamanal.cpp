#include "oracle.hpp"
#include "logging.hpp"
#include "rate_prompt.hpp"

// ggml headers
#include "common.h"

namespace analysis {


//------------------------------------------------------------------------------
// Oracle

bool Oracle::Initialize(const std::string& model_path)
{
    auto lparams = ::llama_context_default_params();

    ContextLength = 2048;

    lparams.n_ctx      = ContextLength;
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
    std::vector<llama_token> tokens = ::llama_tokenize(Context, prompt.c_str(), false);
    const int input_count = static_cast<int>( tokens.size() );

    if (input_count >= ContextLength) {
        BOOST_LOG_TRIVIAL(error) << "Input is too large to fit in the context window. Tokens=" << input_count;
        return false;
    }

    const int NumThreads = 24;

    if (::llama_eval(Context, tokens.data(), tokens.size(), 0, NumThreads)) {
        BOOST_LOG_TRIVIAL(error) << "llama_eval failed";
        return false;
    }

    const int max_output_tokens = 4;

    std::string response;

    for (int i = 0; i < max_output_tokens; ++i)
    {
        const int32_t top_k = 1;
        const float top_p = 0.f;
        const float temp = 0.f;
        const float repeat_penalty = 0.f;

        llama_token id = ::llama_sample_top_p_top_k(Context, nullptr, 0, top_k, top_p, temp, repeat_penalty);

        BOOST_LOG_TRIVIAL(error) << "id[" << i << "] = " << id;

        if (id == llama_token_eos()) {
            BOOST_LOG_TRIVIAL(error) << "EOS";
            break;
        }

        response += ::llama_token_to_str(Context, id);

        bool found = find_first_number_between_0_and_1(response, rating);
        if (found && is_number_complete(response)) {
            BOOST_LOG_TRIVIAL(error) << "found number";
            return true;
        }

        if (i == max_output_tokens - 1) {
            break;
        }

        tokens.push_back(id);

        if (::llama_eval(Context, tokens.data(), tokens.size(), 0, NumThreads)) {
            BOOST_LOG_TRIVIAL(error) << "llama_eval failed";
            return false;
        }
    }

    return find_first_number_between_0_and_1(response, rating);
}


} // namespace analysis
