#pragma once
#include <bitset>
#include <cstdint>
#include <stack>
#include <vector>

static constexpr uint8_t BIT_MASK[9] = {
    0b00000000, // 0 bits
    0b00000001, // 1 bit
    0b00000011, // 2 bits
    0b00000111, // 3 bits
    0b00001111, // 4 bits
    0b00011111, // 5 bits
    0b00111111, // 6 bits
    0b01111111, // 7 bits
    0b11111111  // 8 bits
};

class BitstreamWriter {
    uint8_t ptr = 8;
    uint8_t currentBitstream = 0;

public:
    // Using 8 bit bitstream and implemented as stack because of ANS characteristic (decode is inversed of encode)
    std::vector<uint8_t> bitstream = {};

    BitstreamWriter() = default;

    ~BitstreamWriter() = default;

    /**
     * Write bits on bitstream
     * @param size size of bitstream
     * @param bits bitstream (0bXXX)
     */
    void write(const uint8_t size, const uint8_t bits) {
        uint8_t count = size;
        while (count > 0) {
            const uint8_t sizeToWrite = std::min(count, ptr);
            count -= sizeToWrite;

            const uint8_t bitsToWrite = (bits >> count) & BIT_MASK[size];
            currentBitstream = currentBitstream | (bitsToWrite << (ptr - sizeToWrite));

            ptr -= sizeToWrite;

            if (ptr == 0) {
                bitstream.push_back(currentBitstream);
                currentBitstream = 0;
                ptr = 8;
            }
        }
    }

    /**
     * Flush bitstream
     * Call it when work with bitstream was ended
     * @return offset to read bitstream
     */
    uint8_t flush() {
        uint8_t offset = 0;
        if (ptr < 8) {
            bitstream.push_back(currentBitstream);
            offset = ptr;
            currentBitstream = 0;
            ptr = 8;
        }
        return offset;
    }
};

class BitstreamReader {
    uint64_t buffer = 0;
    uint32_t count = 0;

    /**
     * Load the buffer with 4 bitstream
     */
    void refill() {
        while (count <= 56 && !bitstream.empty()) {
            const uint8_t nextBits = bitstream.back();
            bitstream.pop_back();
            buffer = buffer | (static_cast<uint64_t>(nextBits) << count);
            count += 8;
        }

    }

public:
    std::vector<uint8_t> bitstream = {};

    BitstreamReader() = default;

    explicit BitstreamReader(const std::vector<uint8_t> &bitstream, const std::uint8_t offset) : bitstream(bitstream) {
        this->refill();
        if (offset > 0 && offset < 8) {
            this->advance(offset);
        }
    }

    /**
     * Advance (remove) n bits from bitstream
     * @param n number on bits
     */
    uint32_t advance(const uint32_t n) {
        if (count < n) {
            this->refill();

            //if (count < n) {
            //    throw std::runtime_error("Bitstream underflow (does not have " + std::to_string(n) + " bits to read)");
            //}
        }

        const uint32_t mask = (1U << n) - 1;
        const uint32_t data = buffer & mask;

        buffer = buffer >> n;
        count -= n;

        return data;
    }
};
