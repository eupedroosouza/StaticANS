#pragma once
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

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
    std::map<int8_t, uint16_t> nextStates = {};
    std::map<int8_t, StateBitstream> bitstreams = {};

    State() = default;

    explicit State(const uint16_t state, const std::map<int8_t, uint16_t> &nextStates,
                   const std::map<int8_t, StateBitstream> &bitstreams) : state(state), nextStates(nextStates),
                                                                         bitstreams(bitstreams) {
    }

    ~State() = default;
};

class DecodeState {
public:
    std::int8_t symbol = -1;
    std::map<uint16_t, StateBitstream> states = {};

    DecodeState() = default;

    explicit DecodeState(const std::int8_t symbol) : symbol(symbol) {
    }

    ~DecodeState() = default;
};


class Table {
    std::map<uint16_t, State> states;
    std::map<uint16_t, DecodeState> decodeStates = {};

public:
    explicit Table(const std::initializer_list<State> &states) {
        for (const State &state: states) {
            this->states[state.state] = state;
        }

        for (const auto &[currentState, state]: this->states) {
            for (auto [symbol, nextState]: state.nextStates) {
                if (this->decodeStates.find(nextState) == this->decodeStates.end()) {
                    this->decodeStates[nextState] = DecodeState(symbol);
                }

                if (state.bitstreams.empty()) {
                    this->decodeStates[nextState].states[currentState] = {};
                    continue;
                }

                const StateBitstream &stateBitstream = state.bitstreams.at(symbol);
                this->decodeStates[nextState].states[currentState] = stateBitstream;
            }
        }
    }

    ~Table() = default;

    /**
     * The first state to that table (initialize you state variable with that value)
     * @return the first state
     */
    [[nodiscard]] uint16_t getFirstState() const {
        const auto &[state, _] = *states.begin();
        return state;
    }

    /**
     * Encoded symbol and write on bitstream
     * @param currentState the current state
     * @param symbol the symbol will be encoded
     * @param writer the bitstream writer
     * @return new state (as unsigned 16 bits)
     */
    uint16_t encode(const uint16_t currentState, const int8_t symbol, BitstreamWriter &writer) {
        const State &state = states[currentState];
        const uint16_t &nextState = state.nextStates.at(symbol);
        const StateBitstream &bitstream = state.bitstreams.at(symbol);
        writer.write(bitstream.size, bitstream.bitstream);
        std::cout << "<---->" << std::endl;
        std::cout << "State: " << std::to_string(currentState) << std::endl;
        std::cout << "Next state: " << std::to_string(nextState) << std::endl;
        std::cout << "<---->" << std::endl;
        return nextState;
    }

    /**
     * Decode the simbol and write on bitstream
     * @param currentState the current state
     * @param reader the bitstream reader
     * @return a pair of previous state and symbol value
     */
    std::pair<uint16_t, int8_t> decode(const uint16_t currentState, BitstreamReader &reader) {
        const DecodeState &decodeState = this->decodeStates[currentState];

        // Get major N
        size_t major = 0;
        for (const auto &[previousState,bitstreamWaited]: decodeState.states) {
            if (bitstreamWaited.size > major) {
                major = bitstreamWaited.size;
            }
        }
        if (major == 0) {
            // A state without a bitstream
            auto const &[previousState, _] = *decodeState.states.begin();
            return {previousState, decodeState.symbol};
        }
        std::cout << "<---->" << std::endl;
        std::cout << "State: " << std::to_string(currentState) << std::endl;
        std::cout << "Symbol: " << std::to_string(decodeState.symbol) << std::endl;
        std::cout << "Major: " << std::to_string(major) << std::endl;

        // Read (only peek) N numbers from bitstream
        const auto readBitstreams = reader.peek(static_cast<int>(major));
        std::cout << "readBitstreams: " << std::to_string(readBitstreams) << std::endl;
        // For each previous state possible, check if read bitstream is correct to that state
        for (const auto &[previousState, bitstreamWaited]: decodeState.states) {
            const int n = bitstreamWaited.size;

            if (readBitstreams & bitstreamWaited.bitstream) {
                reader.advance(n);
                return {previousState, decodeState.symbol};
            }
        }
        std::cout << "<---->" << std::endl;

        // Hmmm, is mandatory found a previous state if has a symbols to decode yet, if you are here, check if decodification is correct!
        throw Exception(
            "Not found a previous state to " + std::to_string(currentState) + " (symbol = " + std::to_string(
                decodeState.symbol) + ")");
    }
};
