#pragma once
#include <bitset>
#include <cstdint>
#include <stack>
#include <vector>

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
        for (int i = size - 1; i >= 0; i--) {
            const uint8_t bit = (bits >> i) & 1;

            ptr--;
            currentBitstream = currentBitstream | (bit << ptr);

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
    uint32_t buffer = 0;
    int count = 0;

    /**
     * Load the buffer with 4 bitstream
     */
    void refill() {
        while (count <= 24 && !bitstream.empty()) {
            const uint8_t nextBits = bitstream.back();
            bitstream.pop_back();
            buffer = buffer | (static_cast<uint32_t>(nextBits) << count);
            count += 8;
        }

    }

public:
    std::vector<uint8_t> bitstream;

    explicit BitstreamReader(const std::vector<uint8_t> &bitstream, const std::uint8_t offset) : bitstream(bitstream) {
        this->refill();
        if (offset > 0 && offset < 8) {
            this->advance(offset);
        }
    }

    /**
     * Peek an n bits (just to view, how to check which state to go)
     * @param n number of bits
     * @return bits of bitstream
     */
    uint8_t peek(const int n) {
        if (n > 8) {
            throw Exception("Invalid peek n bits");
        }
        if (count < n) {
            this->refill();
        }
        const uint32_t mask = (1U << n) - 1;
        return buffer & mask;
    }

    /**
     * Advance (remove) n bits from bitstream
     * @param n number on bits
     */
    void advance(const int n) {
        if (count < n) {
            this->refill();
        }

        buffer = buffer >> n;
        count -= n;

        if (count <= 24) {
            refill();
        }
    }

    [[nodiscard]] int getCount() const {
        return count;
    }
};
