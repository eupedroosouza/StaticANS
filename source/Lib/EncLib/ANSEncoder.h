#pragma once
#include "Lib/CommonLib/Context.h"
#include "Lib/CommonLib/ContextModeler.h"



class ANSEncoder {
    Context context;
    ContextModeler contextModeler = ContextModeler();
    uint8_t state = 0;
    BitstreamWriter writer = BitstreamWriter();

    TensorBitwidth tensorBitwidth = {};
    TensorType tensorType = {};

    void encodeBin(uint8_t bin, uint8_t ctxId, TensorType paramType);

    void encodeBinEP(uint8_t bin);

    void encodeBinsEP(uint32_t bins, uint32_t numBins);

    void encodeWeightBAC(int32_t value, uint8_t k);

    void encodeAbsRem(int32_t value, uint16_t k);

    void encodeWeightsChunks(const int32_t *pWeights, uint32_t numWeights);

public:
    ANSEncoder() = default;

    explicit ANSEncoder(const Context &context);

    ~ANSEncoder() = default;

    void setBitwidthAndType(const TensorBitwidth bitwidth, const TensorType type) {
        tensorBitwidth = bitwidth;
        tensorType = type;
    }

    void iae_v(uint8_t bitwidth, int32_t value);

    void uae_v(uint8_t bitwidth, uint32_t value);

    void encodeTensorHeader(const uint32_t *shape, uint32_t numDims, uint16_t tensorId);

    void encodeWeights(const int32_t *pWeights, uint32_t numWeights);

    std::vector<uint8_t>& finishEncoding();

    static constexpr uint32_t MAX_TENSORS_BITS = 12; // allows up to 4096 tensors
    static constexpr uint32_t chunkSize = 2048;
};
