#pragma once
#include <algorithm>
#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <utility>

#include "TypeDef.h"
#include "Lib/Utils/Bitstream.h"


class StateBitstream {
public:
    uint8_t size = 0;
    uint8_t bitstream = 0;

    StateBitstream() = default;

    explicit StateBitstream(const uint8_t size, const uint8_t bitstream) : size(size), bitstream(bitstream) {
    }

    ~StateBitstream() = default;
};

class State {
public:
    uint16_t state = 0;
    std::vector<uint16_t> nextStates = {};
    std::vector<StateBitstream> bitstreams = {};

    State() = default;

    explicit State(const uint16_t state, const std::vector<uint16_t> &nextStates,
                   const std::vector<StateBitstream> &bitstreams) : state(state), nextStates(nextStates),
                                                                    bitstreams(bitstreams) {
    }

    ~State() = default;
};

class DecodeState {
public:
    std::int8_t symbol = -1;
    std::uint8_t N = 0;
    std::uint16_t base = 0;

    DecodeState() = default;

    explicit DecodeState(const std::int8_t symbol, const std::uint8_t N, const std::uint16_t base) : symbol(symbol),
        N(N),
        base(base) {
    }

    ~DecodeState() = default;
};


class Table {
    uint16_t L = 0;

    std::vector<State> states = {};
    std::vector<DecodeState> decodeStates = {};

public:
    Table() = default;

    explicit Table(const std::list<State> &states) {
        // Temporary (.dat needs has an L number)
        L = states.front().state;
        for (const State &state: states) {
            this->L = std::min(this->L, state.state);
        }

        this->states.resize(L);
        for (const State &state: states) {
            this->states[state.state - L] = state;
        }

        this->decodeStates.resize(L);
        for (const auto &state: this->states) {
            for (int8_t symbol = 0; symbol < static_cast<int8_t>(state.nextStates.size()); ++symbol) {
                const uint16_t &nextState = state.nextStates[symbol];
                if (state.bitstreams.empty() && this->decodeStates[nextState - L].symbol == -1) {
                    this->decodeStates[nextState - L] = DecodeState(symbol, 0, state.state);
                    continue;
                }
                const StateBitstream &stateBitstream = state.bitstreams.at(symbol);
                if (this->decodeStates[nextState - L].symbol == -1) {
                    this->decodeStates[nextState - L] = DecodeState(symbol, stateBitstream.size, state.state);
                } else {
                    DecodeState &decodeState = this->decodeStates[nextState - L];
                    decodeState.base = std::min(decodeState.base, state.state);
                }
            }
        }
    }

    ~Table() = default;

    /**
     * The first state to that table (initialize you state variable with that value)
     * @return the first state
     */
    [[nodiscard]] uint16_t getFirstState() const {
        return L;
    }

    /**
     * Encoded symbol and write on bitstream
     * @param currentState the current state
     * @param symbol the symbol will be encoded
     * @param writer the bitstream writer
     * @return new state (as unsigned 16 bits)
     */
    uint16_t encode(const uint16_t currentState, const int8_t symbol, BitstreamWriter &writer) {
        const State &state = states[currentState - L];
        const uint16_t &nextState = state.nextStates.at(symbol);
        const StateBitstream &bitstream = state.bitstreams.at(symbol);
        writer.write(bitstream.size, bitstream.bitstream);
        return nextState;
    }

    /**
     * Decode the simbol and write on bitstream
     * @param currentState the current state
     * @param reader the bitstream reader
     * @return a pair of previous state and symbol value
     */
    std::pair<uint16_t, int8_t> decode(const uint16_t currentState, BitstreamReader &reader) {
        const DecodeState &decodeState = this->decodeStates[currentState - L];

        // A state without a bitstream
        if (decodeState.N == 0) {
            return {decodeState.base, decodeState.symbol};
        }

        const auto readBitstreams = reader.peek(decodeState.N);
        reader.advance(decodeState.N);
        const auto previousState = decodeState.base + readBitstreams;
        return {previousState, decodeState.symbol};
    }
};
