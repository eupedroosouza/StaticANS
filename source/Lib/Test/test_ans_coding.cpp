#include <array>
#include <iostream>

#include "Lib/CommonLib/Context.h"
#include "Lib/CommonLib/TabledANS.h"
#include "Lib/CommonLib/UniformRangeANS.h"

void testEPRANS() {
    std::cout << "<<=== Equiprobable range-ANS ===>>" << std::endl;
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

    std::string bitstream;
    const std::stack<uint8_t> viewBitstream = writer.bitstream;
    BitstreamReader viewReader(viewBitstream, offset);
    while (viewReader.getCount() != 0) {
        const uint8_t bit = viewReader.peek(1);
        viewReader.advance(1);
        bitstream += std::to_string(bit);
    }
    std::cout << "Bitstream: " << bitstream << std::endl;

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
}

void testTANS() {
    std::cout << "<<=== tabled-ANS ===>>" << std::endl;

    Context ctx = Context::loadContextFromFile("../static/tables.dat");
    std::cout << "Loaded tables" << std::endl;

    std::cout << "=== Encoding ===" << std::endl;
    Table *table = ctx.getContextTable(0, TensorType::Weight);
    BitstreamWriter writer;
    uint16_t state = table->getFirstState();
    state = table->encode(state, 0, writer);
    state = table->encode(state, 1, writer);
    state = table->encode(state, 1, writer);
    state = table->encode(state, 0, writer);
    state = table->encode(state, 1, writer);
    state = table->encode(state, 1, writer);
    state = table->encode(state, 1, writer);
    state = table->encode(state, 1, writer);
    state = table->encode(state, 1, writer);
    state = table->encode(state, 0, writer);
    state = table->encode(state, 0, writer);
    state = table->encode(state, 1, writer);
    const int offset = writer.flush();

    std::cout << "Final state: " << state << "." << std::endl;

    std::string bitstream;
    const std::stack<uint8_t> viewBitstream = writer.bitstream;
    BitstreamReader viewReader(viewBitstream, offset);
    while (viewReader.getCount() != 0) {
        const uint8_t bit = viewReader.peek(1);
        viewReader.advance(1);
        bitstream += std::to_string(bit);
    }
    std::cout << "Bitstream: " << bitstream << std::endl;

    std::cout << "=== Decoding ===" << std::endl;
    std::string decodedSymbols;
    BitstreamReader reader(writer.bitstream, offset);
    uint16_t decodedState = state;
    for (int i = 17; i > 0; i--) {
        auto [previosState, symbol] = table->decode(decodedState, reader);
        decodedState = previosState;
        if (i == 1) {
            decodedSymbols.append(std::to_string(symbol));
            break;
        }
        decodedSymbols.append(std::to_string(symbol) + ", ");
    }
    std::cout << "Decoded symbols: " << decodedSymbols << std::endl;
}

int main() {

    testEPRANS();
    std::cout << std::endl;
    testTANS();

    return 0;
}
