#include "deepANS.h"

Encoder::Encoder(const Context &context) {
    this->encoder = ANSEncoder(context);
}

void Encoder::iae_v(const uint8_t bitwidth, const int32_t value) {
    encoder.iae_v(bitwidth, value);
}

void Encoder::uae_v(const uint8_t bitwidth, const uint32_t value) {
    encoder.uae_v(bitwidth, value);
}

void Encoder::encodeLayer(const TensorMeta &tensor, const uint16_t tensorId) {
    const uint32_t numWeights = tensor.data.size();

    encoder.setBitwidthAndType(tensor.tensorBitwidth, tensor.tensorType);

    encoder.encodeWeights(tensor.data.data(), numWeights);
    encoder.encodeTensorHeader(tensor.shape.data(), tensor.numDims, tensorId);
}

std::vector<uint8_t> Encoder::encodeModel(const std::vector<TensorMeta> &modelTensors) {
    const uint32_t numTensors = modelTensors.size();
    for (uint16_t tensorId = 0; tensorId < numTensors; tensorId++) {
        encodeLayer(modelTensors[tensorId], tensorId);
    }
    uae_v(ANSEncoder::MAX_TENSORS_BITS, numTensors);
    return this->finishEncoding();
}

std::vector<uint8_t> Encoder::finishEncoding() {
    return encoder.finishEncoding();
}
