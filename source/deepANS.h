#pragma once
#include <cstdint>
#include <vector>

#include "Lib/CommonLib/TypeDef.h"

class Encoder {
    std::vector<uint8_t> data;

public:
    ~Encoder() = default;

    /**
     * Encode model
     * @param modelTensors loaded tensors
     * @return encoded bytestream
     */
    const std::vector<uint8_t> &encodeModel(const std::vector<TensorMeta> &modelTensors);
};

class Decoder {
public:
    explicit Decoder(std::vector<uint8_t> &data);

    ~Decoder() = default;


};
