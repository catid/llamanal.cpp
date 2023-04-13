# Not functional.

This is a work in progress.  I got it to a certain point where it's querying the llama.cpp LLM, and hit an unexpected wall: The CPU version seems to take a lot of time proportional to the input size, while the GPU version I have been using does not take time proportional to the input size.  So, it's far too slow!  This seems like a bug.

For example in my supercharger repo this function does the heavy lifting:

                    out = self.model(input_ids=input_tensor,
                                use_cache=True,
                                attention_mask=attention_mask,
                                past_key_values=past_key_values)

This is the time in seconds to generate each output value:

```
TEST: 0.1767594814300537
TEST: 0.25008654594421387
TEST: 0.23602938652038574
TEST: 0.23096466064453125
TEST: 0.23759722709655762
```

If I replace the long input prompt with one that just says "Hello", the time taken is not dramatically increased:

```
TEST: 0.14011812210083008
TEST: 0.1371593475341797
TEST: 0.13875913619995117
TEST: 0.139174222946167
TEST: 0.14201641082763672
```

So, I was expecting the CPU version to follow the same behavior, but it seems like something is very different here.


# LLaMA.cpp Static Code Analysis Example

Static code analysis using llama.cpp and the best LLM you can run offline without an expensive GPU.

Want to leverage AI to scan your codebase for bugs, but are unable to upload your files to Microsoft?  Here's the next best thing!

![analysis logo](analysis.jpg)

## How does it work?  What's the idea?

Based on the observation that LLMs only cost CPU time per output token, so it is very cheap to get a short 0..1 rating for a piece of code, even if the model is 65B parameters.

Only 30B and 65B models are large enough to do bug finding in complex code.  See example here: https://twitter.com/MrCatid/status/1646054575533699075?s=20

I'd recommend running this on a computer with 64GB of RAM.

## Setup

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y git cmake g++ make libboost-all-dev

# For speed
sudo apt-get install libopenblas-dev

# For C++ analysis
sudo apt-get install libclang-dev clang-15

# Clone repo
git clone https://github.com/catid/llamanal.cpp
cd ./llamanal.cpp/

# Download 65B model
sudo apt install git-lfs
git lfs install
git clone https://huggingface.co/CRD716/ggml-LLaMa-65B
mv ggml-LLaMa-65B/ggml-LLaMa-65B-q4_0.bin ./models/
rm -rf ggml-LLaMa-65B

# Build the software
mkdir build
cd build
cmake -DLLAMA_OPENBLAS=ON ..
make -j

# Test it out on its own code
./bin/analysis ..
```

## Future Work

* Add support for smaller models.
* Support for other languages can be added without refactoring.
* Can be merged into llama.cpp repo somewhat painlessly.
