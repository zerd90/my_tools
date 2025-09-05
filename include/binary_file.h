
#ifndef Z_BIN_FILE_H
#define Z_BIN_FILE_H

#include <string.h>

#include <iostream>
#include <memory>

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

void splitpath(std::string &path, std::string &drv, std::string &dir, std::string &name, std::string &ext);
uint64_t get_file_size(FILE *fp);


struct DataBlock
{
	uint64_t length;
	std::shared_ptr<uint8_t[]> data;

	DataBlock() : length(0) {}
	explicit DataBlock(uint64_t len) : data(new uint8_t[len])
	{
		length = len;
	}

	~DataBlock() {}

	DataBlock &operator=(const DataBlock &) = delete;

	void create(uint64_t len)
	{
		data = std::shared_ptr<uint8_t[]>(new uint8_t[len]);
		length = len;
	}

	uint8_t *ptr()
	{
		return this->data.get();
	}

};

struct BinaryReader
{
	std::string fn;
	std::string drv;
	std::string path;
	std::string base_name;
	std::string ext;

	FILE *fp = NULL;

	bool opened = false;

	uint64_t file_size = 0;
	uint64_t _read_pos = 0;

private:
	static const uint64_t buffer_size = 1024 * 1024;
	uint8_t *read_buffer;
	uint64_t buffer_start_pos;
	uint64_t buffer_contain_size;

private:
	int check_buffer(uint64_t read_psos, uint64_t read_size);

public:
	BinaryReader()
	{
		read_buffer = new uint8_t[buffer_size];
	}

	explicit BinaryReader(std::string &fn) : BinaryReader()
	{
		open(fn);
	}

	~BinaryReader()
	{
		if (opened)
			close();
		delete[] read_buffer;
	}

	int open(std::string &fn);

	int close();

	uint64_t set_file_cursor(uint64_t abs_offset);
	uint64_t read(void *buf, uint64_t len);

	uint64_t jump_read(uint64_t pos, void *buf, uint64_t len); // read len from pos, then back to the _read_pos before

	uint64_t read_still(void *buf, uint64_t len);

	std::string read_str( uint64_t max_len);

	uint64_t read_data(void *buf, uint16_t buf_len, uint16_t data_len, bool reverse);

	uint8_t read_u8();
	int8_t read_s8();
	uint16_t read_u16(bool reverse);
	int16_t read_s16(bool reverse);
	uint32_t read_u32(bool reverse);
	int32_t read_s32(bool reverse);
	uint64_t read_u64(bool reverse);
	int64_t read_s64(bool reverse);

	uint64_t read_un(uint16_t bytes, bool reverse);
	int64_t read_sn(uint16_t bytes, bool reverse);
	uint64_t set_cursor(uint64_t pos);
} ;


#endif
