#ifndef Z_BITS_H
#define Z_BITS_H

#include <stdint.h>

#ifdef __linux
    #include <byteswap.h>
#else
    #define bswap_16(n) (((n) << 8) | ((n) >> 8))

    #define bswap_32(n) (((n) << 24) | (((n) << 8) & 0xff0000) | (((n) >> 8) & 0xff00) | ((n) >> 24))

    #define bswap_64(n) ((uint64_t)bswap_32((uint32_t)(n)) << 32 | bswap_32((uint32_t)((n) >> 32)))

#endif
struct BitsReader
{
    uint8_t *buf;
    uint32_t size_bits = 0;
    uint32_t ptr       = 0;
    BitsReader()       = delete;
    BitsReader(void *buf, uint32_t size_bytes)
    {
        this->buf = (uint8_t *)buf;
        size_bits = size_bytes * 8;
        ptr       = 0;
    }
    ~BitsReader() {}

    uint8_t  read_bit();
    uint32_t read_bit(int bits_cnt);
    uint32_t read_golomb();
};

struct BitsWriter
{
    uint8_t *buf;
    uint32_t size_bits = 0;
    uint32_t ptr       = 0;
    BitsWriter()       = delete;
    BitsWriter(void *buf, uint32_t size_bytes)
    {
        this->buf = (uint8_t *)buf;
        size_bits = size_bytes * 8;
        ptr       = 0;
    }
    ~BitsWriter() {}

    void write_bit(uint8_t val);
    void write_bit(int bits_cnt, uint64_t val);
};

#endif