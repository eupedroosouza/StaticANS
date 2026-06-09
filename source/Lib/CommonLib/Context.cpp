#include  "Context.h"

#include <algorithm>
#include <fstream>
#include <list>

Context Context::loadContextFromFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw Exception("Could not open file: " + filename);
    }

    uint32_t size;
    file.read(reinterpret_cast<char *>(&size), sizeof(size));

    std::array<std::array<Table, 13>, 2> ctxTables = {};
    for (uint32_t i = 0; i < size; i++) {
        uint8_t type;
        file.read(reinterpret_cast<char *>(&type), sizeof(type));
        uint8_t id;
        file.read(reinterpret_cast<char *>(&id), sizeof(id));
        uint8_t symbolsSize;
        file.read(reinterpret_cast<char *>(&symbolsSize), sizeof(symbolsSize));
        uint16_t total;
        file.read(reinterpret_cast<char *>(&total), sizeof(total));
        std::list<State> states;
        const uint16_t range = static_cast<uint16_t>(2 * total) - 1;
        for (uint16_t state = total; state <= range; state++) {
            std::vector<uint16_t> nextStates = {};
            nextStates.resize(symbolsSize);
            std::vector<StateBitstream> bitstreams = {};
            bitstreams.resize(symbolsSize);
            for (uint8_t symbol = 0; symbol < symbolsSize; symbol++) {
                uint16_t nextState;
                file.read(reinterpret_cast<char *>(&nextState), sizeof(nextState));
                uint8_t bitstreamSize;
                file.read(reinterpret_cast<char *>(&bitstreamSize), sizeof(bitstreamSize));
                uint8_t bitstream;
                file.read(reinterpret_cast<char *>(&bitstream), sizeof(bitstream));
                const auto stateBitstream = StateBitstream(bitstreamSize, bitstream);
                nextStates[static_cast<int8_t>(symbol)] = nextState;
                bitstreams[static_cast<int8_t>(symbol)] = stateBitstream;
            }
            states.emplace_back(state, nextStates, bitstreams);
        }

        const auto table = Table(states);
        ctxTables[type][id] = table;
    }

    return Context(ctxTables);
}
