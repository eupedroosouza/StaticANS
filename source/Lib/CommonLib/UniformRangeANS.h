#pragma once
#include <cstdint>

#include "Lib/Utils/Bitstream.h"

class UniformRangeANS {
    uint8_t m = 0;

public:
    UniformRangeANS() = default;

    explicit UniformRangeANS(const uint8_t m) : m(m) {
    }

    ~UniformRangeANS() = default;

    /**
    * The first state to that table (initialize you state variable with that value)
    * @return the first state
    */
    [[nodiscard]] uint16_t getFirstState() const {
        return static_cast<int16_t>(m);
    }

    /**
     * Encoded equiprobable symbol and write on bitstream
     * @param currentState the current state
     * @param symbol the symbol will be encoded
     * @param writer the bitstream writer
     * @return  new state (as unsigned 16 bits)
     */
    uint16_t encode(uint16_t &currentState, const int8_t symbol, BitstreamWriter &writer) const {
        while (currentState >= m) {
            const auto bit = static_cast<uint8_t>(currentState % m);
            writer.write(1, bit);
            currentState /= m;
        }

        return (currentState * m) + symbol;
    }

    /**
     * Decoded equiprobable symbol
     * @param currentState the current state
     * @param reader the bitstream reader
     * @return a pair with previous state and decoded symbol
     */
    std::pair<uint16_t, uint8_t> decode(const uint16_t currentState, BitstreamReader &reader) const {
        const auto symbol = static_cast<int8_t>(currentState % m);
        auto previousState = static_cast<uint16_t>(currentState / m);

        while (previousState < m) {
            const uint8_t bit = reader.peek(1);
            reader.advance(1);
            previousState = static_cast<uint16_t>((previousState * m) + bit);
        }

        return {previousState, symbol};
    }
};
