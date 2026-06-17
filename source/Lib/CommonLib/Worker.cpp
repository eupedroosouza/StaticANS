#include "Worker.h"

#include <thread>

WorkerEncoder::WorkerEncoder(const int numWorkers, const Context &context,
                             const std::vector<TensorMeta> &modelTensors) {
    this->numWorkers = numWorkers;
    this->context = context;
    this->modelTensors = modelTensors;
    this->encodedTensors.resize(modelTensors.size());
}

void WorkerEncoder::run() {
    while (!modelTensors.empty()) {
        TensorMeta &tensor = modelTensors.back();
        modelTensors.pop_back();
        auto encoder = ANSEncoder(context);

        // Encode Layer
        const uint32_t numWeights = tensor.data.size();

        encoder.setBitwidthAndType(tensor.tensorBitwidth, tensor.tensorType);

        encoder.encodeWeights(tensor.data.data(), numWeights);
        encoder.encodeTensorHeader(tensor.shape.data(), tensor.numDims, tensor.tensorId);
        // End encode layer

        this->encodedTensors[tensor.tensorId] = encoder.finishEncoding();
    }
}

std::vector<uint8_t> WorkerEncoder::encodeModel() {
    std::vector<uint8_t> bytestream = {};

    std::vector<std::thread> threads = {};
    threads.resize(this->numWorkers);
    for (int i = 0; i < this->numWorkers; i++) {
        std::thread thread([this] {
            run();
        });
        threads.push_back(std::move(thread));
    }

    for (std::thread &thread: threads) {
        thread.join();
    }

    for (auto tensorBytestream : this->encodedTensors) {
        bytestream.push_back(tensorBytestream.size()); // todo: 12-bits also
        bytestream.insert(bytestream.end(), tensorBytestream.begin(), tensorBytestream.end());
    }

    //  todo: 12-bit value, use 16 bits (2 bytes) to save tensor size (apply BitUtil here)
    bytestream.push_back(modelTensors.size());
    return bytestream;
}

WorkerDecoder::WorkerDecoder(const int numWorkers) : numWorkers(numWorkers) {
};
