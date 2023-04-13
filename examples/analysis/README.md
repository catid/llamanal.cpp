# LLaMA.cpp Static Code Analysis Example

Static code analysis using llama.cpp and the best LLM you can run offline without an expensive GPU.

Want to leverage AI to scan your codebase for bugs, but are unable to upload your files to Microsoft?  Here's the next best thing!

![analysis logo](analysis.jpg)

## How does it work?  What's the idea?

Example:

```bash
./analyze_cpp bad_code.cpp
FIXME: Output is terrible!
```

Based on the observation that LLMs only cost CPU time per output token, so it is very cheap to get a short 0..1 rating for a piece of code, even if the model is 65B parameters.

Only 30B and 65B models are large enough to do bug finding in complex code.  See example here: https://twitter.com/MrCatid/status/1646054575533699075?s=20

I'd recommend running this on a computer with 64GB of RAM.

## Setup

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y git cmake g++ make libboost-all-dev

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
cmake ..
make -j
```

## Future Work

* Add support for smaller models.
* Support for other languages can be added without refactoring.
* Can be merged into llama.cpp repo somewhat painlessly.
