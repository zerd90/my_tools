
#include "bits.h"
uint8_t BitsReader::read_bit()
{
	if (ptr >= size_bits)
		return 0;

	uint32_t byte_ptr = ptr / 8;
	uint8_t byte = *(buf + byte_ptr);

	uint8_t bits_ptr = 7 - ptr % 8;
	uint8_t res = (byte >> bits_ptr) & 0x1;

	ptr++;
	return res;
}

uint32_t BitsReader::read_bit(int bits_cnt)
{
	uint32_t res = 0;
	for (int i = 0; i < bits_cnt; i++)
	{
		uint8_t bit = read_bit();
		res = (res << 1) + bit;
	}

	return res;
}

uint32_t BitsReader::read_golomb()
{
    int n;

    for (n = 0; read_bit() == 0 && ptr < size_bits; n++)
        ;

    return ((uint64_t)1 << n) + read_bit(n) - 1;
}

void BitsWriter::write_bit(uint8_t val)
{
	if (ptr >= size_bits)
		return;

	val = val & 0x1;

	uint32_t byte_ptr = ptr / 8;
	uint8_t byte = *(buf + byte_ptr);

	uint8_t bits_ptr = 7 - ptr % 8;
	uint8_t res = (byte & (~(0x01 << bits_ptr))) + (val << bits_ptr);

	*(buf + byte_ptr) = res;
	ptr++;
}

void BitsWriter::write_bit(int bits_cnt, uint64_t val)
{
	for (int i = 0; i < bits_cnt; i++)
	{
		uint8_t bit = (val >> (bits_cnt - 1 - i)) & 0x1;
		write_bit(bit);
	}
}

