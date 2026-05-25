#include <array>
#include <iostream>

#include "Lib/CommonLib/TabledANS.h"
#include "Lib/CommonLib/UniformRangeANS.h"

int main() {
    std::cout << "=== Encoding ===" << std::endl;

    BitstreamWriter writer;
    const UniformRangeANS ep(2);
    uint16_t state = ep.getFirstState();
    state = ep.encode(state, 0, writer);
    state = ep.encode(state, 0, writer);
    state = ep.encode(state, 1, writer);
    state = ep.encode(state, 0, writer);
    state = ep.encode(state, 1, writer);
    state = ep.encode(state, 1, writer);
    state = ep.encode(state, 0, writer);
    state = ep.encode(state, 0, writer);
    state = ep.encode(state, 0, writer);
    state = ep.encode(state, 0, writer);
    state = ep.encode(state, 0, writer);
    state = ep.encode(state, 1, writer);
    state = ep.encode(state, 1, writer);
    state = ep.encode(state, 1, writer);
    state = ep.encode(state, 1, writer);
    state = ep.encode(state, 1, writer);
    state = ep.encode(state, 1, writer);
    state = ep.encode(state, 1, writer);
    state = ep.encode(state, 1, writer);
    state = ep.encode(state, 1, writer);
    state = ep.encode(state, 1, writer);

    const uint8_t offset = writer.flush();
    std::cout << "Final state: " << state << "." << std::endl;

    std::cout << "=== Decoding ===" << std::endl;
    std::string decodedSymbols;
    BitstreamReader reader(writer.bitstream, offset);
    uint16_t decodedState = state;
    for (int i = 21; i > 0; i--) {
        auto [previosState, symbol] = ep.decode(decodedState, reader);
        decodedState = previosState;
        if (i == 1) {
            decodedSymbols.append(std::to_string(symbol));
            break;
        }
        decodedSymbols.append(std::to_string(symbol) + ", ");
    }
    std::cout << "Decoded symbols: " << decodedSymbols << std::endl;

    return 0;
}
