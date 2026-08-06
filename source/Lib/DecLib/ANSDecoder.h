#pragma once
#include "Lib/CommonLib/Context.h"
#include "Lib/CommonLib/ContextModeler.h"
#include "Lib/CommonLib/TypeDef.h"
#include "Lib/Utils/Bitstream.h"

class Context;

class ANSDecoder {
    BitstreamReader reader = {};

    Context context;
    ContextModeler contextModeler = ContextModeler();
    uint8_t contextualizedStates[2][13] = {};
    uint8_t equiprobableState = 0;

    TensorBitwidth tensorBitwidth = {};
    TensorType tensorType = {};

    uint32_t decodeBin(uint8_t ctxId, TensorType paramType);

    uint32_t decodeBinEP();

    uint32_t decodeBinsEP(uint32_t numBins);

    void decodeWeightsChunks(int32_t *pWeights, uint32_t numWeights);

    void decodeWeightVal(int32_t &decodedIntVal, uint8_t k);

    void decodeAbsRem(uint32_t &remainder, uint32_t k);

public:
    ANSDecoder() = default;

    explicit ANSDecoder(const Context &context, std::vector<uint8_t> &bytestream);

    ~ANSDecoder() = default;

    int32_t iae_v(uint8_t bitwidth);

    uint32_t uae_v(uint8_t bitwidth);

    void decodeTensorHeader(uint32_t *shape, uint32_t &numDims, TensorMeta &tensor);

    void decodeWeights(int32_t *pWeights, uint32_t numWeights);
};
