#pragma once
#include "Context.h"
#include "TypeDef.h"
#include "Lib/EncLib/ANSEncoder.h"

class WorkerEncoder {
    int numWorkers = {};
    Context context = {};

    std::vector<TensorMeta> modelTensors = {};
    std::vector<std::vector<uint8_t>> encodedTensors = {};

    void run();

public:
    WorkerEncoder() = default;

    explicit WorkerEncoder(int numWorkers, const Context &context, const std::vector<TensorMeta> &modelTensors);

    ~WorkerEncoder() = default;

    std::vector<uint8_t> encodeModel();
};

class WorkerDecoder {
    int numWorkers;

public:
    WorkerDecoder() = default;

    explicit WorkerDecoder(int numWorkers);

    ~WorkerDecoder() = default;

    void decodeModel(std::vector<TensorMeta> &modelTensors);
};
