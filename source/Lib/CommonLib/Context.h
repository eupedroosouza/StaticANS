#pragma once

#include <array>

#include "TabledANS.h"

class Context {
    // 0 for weights, 1 for bias
    // 13 different contexts
    std::array<std::array<Table, 13>, 2> context = {};

public:
    Context() = default;

    explicit Context(const std::array<std::array<Table, 13>, 2> &context) : context(context) {
    }

    ~Context() = default;

    Table *getContext(const uint8_t ctxId, const TensorType paramType) {
        const int type = paramType == TensorType::Weight ? 0 : 1;
        return &context[type][ctxId];
    }

    std::array<std::array<Table, 13>, 2>& getContexts() {
        return  context;
    }

    static Context loadContextFromFile(const std::string &filename);

};
