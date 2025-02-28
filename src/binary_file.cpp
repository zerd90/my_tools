

#include <sys/stat.h>
#include <stdio.h>
#include <string>
using std::string;

#ifdef _MSC_VER
    #include <io.h>
    #define access _access
    #define F_OK (00)
    #define W_OK (02)
    #define R_OK (04)
    #define X_OK (06)

    #define bswap_16(n) ((n << 8) | (n >> 8))
    #define bswap_32(n) ((n << 24) | ((n << 8) & 0xff0000) | ((n >> 8) & 0xff00) | (n >> 24))
    #define bswap_64(n) ((uint64_t)bswap_32((uint32_t)n) << 32 | bswap_32((uint32_t)(n >> 32)))
#else
    #include <unistd.h>
    #ifdef __linux
        #include <byteswap.h>
    #else

        #define bswap_16(n) ((n << 8) | (n >> 8))
        #define bswap_32(n) ((n << 24) | ((n << 8) & 0xff0000) | ((n >> 8) & 0xff00) | (n >> 24))
        #define bswap_64(n) ((uint64_t)bswap_32((uint32_t)n) << 32 | bswap_32((uint32_t)(n >> 32)))
    #endif
#endif

#include "binary_file.h"
#include "logger.h"

#if defined(WIN32) || defined(_WIN32)
    #define fseek64 _fseeki64
    #define ftell64 _ftelli64
using file_stat64_t = struct _stat64;
    #define stat64 _stat64

#elif defined(__linux)
    #define fseek64 fseeko64
    #define ftell64 ftello64
using file_stat64_t = struct stat64;
#else // Mac
    #define fseek64 fseek
    #define ftell64 ftell
    #define stat64 stat
using file_stat64_t = struct stat;
#endif

void splitpath(std::string &path, std::string &drv, std::string &dir, std::string &name, std::string &ext)
{
    uint64_t drv_pos  = path.find(':');
    uint64_t name_pos = path.rfind('/');
    if (string::npos == name_pos)
        name_pos = path.rfind('\\');
    uint64_t ext_pos = path.rfind('.');

    if (drv_pos != string::npos)
        drv = path.substr(0, ++drv_pos);
    else
        drv_pos = 0;

    if (name_pos != string::npos)
        dir = path.substr(drv_pos, ++name_pos - drv_pos);
    else
        name_pos = 0;

    if (ext_pos != string::npos)
        ext = path.substr(ext_pos, path.length() - ext_pos);
    else
        ext_pos = path.length();

    name = path.substr(name_pos, ext_pos - name_pos);
}

uint64_t get_file_size(FILE *fp)
{
    uint64_t cur = ftell64(fp);
    fseek64(fp, 0, SEEK_END);
    uint64_t len = ftell64(fp);
    fseek64(fp, cur, SEEK_SET);
    return len;
}

int BinaryReader::check_buffer(uint64_t read_pos, uint64_t read_size)
{
    if (read_pos >= fileSize)
    {
        return -1;
    }

    uint64_t can_rd_sz = MIN(read_size, fileSize - read_pos);

    if (buffer_start_pos > read_pos || read_pos + can_rd_sz > buffer_start_pos + buffer_contain_size)
    {
        buffer_contain_size = MIN(fileSize - read_pos, buffer_size);
        if (fseek64(fp, read_pos, SEEK_SET) < 0)
        {
            Z_ERR("seek to {#x} fail {}\n", read_pos, strerror(errno));
            exit(0);
        }
        if (fread(read_buffer, 1, buffer_contain_size, fp) != buffer_contain_size)
        {
            Z_ERR("read {} fail({}), pos {}, size {}\n", fn, strerror(errno), read_pos, buffer_contain_size);
            exit(0);
        }
        buffer_start_pos = read_pos;
        Z_DBG("update buffer, pos={#x}, size={}\n", buffer_start_pos, buffer_contain_size);
    }
    return 0;
}

int BinaryReader::open(std::string &newFileName)
{
    int   ret = 0;
    FILE *tmpfp;

    ret = access(newFileName.c_str(), F_OK | R_OK);
    if (ret != 0)
    {
        Z_ERR("file ");
        Log::blue("{}", newFileName);
        Log::print(" can't access\n");
        goto exit;
    }

    file_stat64_t file_state;
    ret = stat64(newFileName.c_str(), &file_state);
    if (ret < 0)
    {
        Z_ERR("Can't state file ");
        Log::blue("{}\n", newFileName);
        goto exit;
    }
    if (file_state.st_mode & S_IFDIR)
    {
        Log::blue("{}", newFileName);
        Log::print(" is a dir\n");
        ret = -1;
        goto exit;
    }

    tmpfp = fopen(newFileName.c_str(), "rb");
    if (!tmpfp)
    {
        Z_ERR("Can't open file");
        Log::blue("{}\n", newFileName);
        ret = -1;
        goto exit;
    }

    if (opened)
        close();

    fp = tmpfp;

    fileSize = get_file_size(fp);
    _read_pos = 0;

    fn = newFileName;

    splitpath(fn, drv, path, base_name, ext);

    opened = true;

    if (read_buffer)
    {
        buffer_contain_size = MIN(fileSize, buffer_size);
        size_t rd_size      = fread(read_buffer, 1, buffer_contain_size, fp);
        if (rd_size != buffer_contain_size)
        {
            Z_ERR("read err {} != {}\n", rd_size, buffer_contain_size);
        }
        buffer_start_pos = 0;
    }

exit:
    return ret;
}

int BinaryReader::close()
{
    opened = false;

    if (!fp)
    {
        Z_ERR("{} not opened", fn);
        return -1;
    }

    int ret = fclose(fp);
    if (ret < 0)
    {
        Z_ERR("{} close fail({})", fn, ret);
    }
    fp = nullptr;
    return ret;
}

uint64_t BinaryReader::read(void *buf, uint64_t len)
{
    if (!buf)
    {
        _read_pos = MIN(_read_pos + len, fileSize);
        return 0;
    }

    uint64_t rd_size = read_still(buf, len);

    _read_pos += rd_size;

    return rd_size;
}

uint64_t BinaryReader::jump_read(uint64_t pos, void *buf, uint64_t len)
{
    set_file_cursor(pos);
    uint64_t ret = fread(buf, 1, len, fp);

    return ret;
}

uint64_t BinaryReader::read_still(void *buf, uint64_t len)
{
    uint64_t rd_size;

    if (len <= buffer_size)
    {
        if (check_buffer(_read_pos, len) < 0)
        {
            return 0;
        }

        rd_size = MIN(len, buffer_contain_size - (_read_pos - buffer_start_pos));
        memcpy(buf, read_buffer + (_read_pos - buffer_start_pos), len);
    }
    else
    {
        fseek64(fp, _read_pos, SEEK_SET);
        rd_size = fread(buf, 1, len, fp);
    }

    return rd_size;
}

string BinaryReader::read_str(uint64_t max_len)
{
    string dst_string;

    if (max_len > fileSize - _read_pos)
    {
        max_len = fileSize - _read_pos;
    }

    char     c       = 0;
    uint64_t max_end = _read_pos + max_len;

    dst_string.clear();
    while (1)
    {
        if (_read_pos >= max_end)
        {
            break;
        }
        read(&c, 1);

        if (c == 0)
            break;
        dst_string.push_back(c);
    }

    return dst_string;
}

uint64_t BinaryReader::read_data(void *buf, uint16_t buf_len, uint16_t data_len, bool reverse)
{
    uint64_t ret = 0;
    if (reverse)
    {
        int pos = buf_len - data_len;
        ret     = read((uint8_t *)buf + pos, data_len);
        if (ret < data_len)
            return 0;

        switch (buf_len)
        {
            case 2:
                *(uint16_t *)buf = bswap_16(*(uint16_t *)buf);
                break;
            case 4:
                *(uint32_t *)buf = bswap_32(*(uint32_t *)buf);
                break;
            case 8:
                *(uint64_t *)buf = bswap_64(*(uint64_t *)buf);
                break;
            default:
                break;
        }
    }
    else
    {
        ret = read(buf, data_len);
    }

    return ret;
}

uint8_t BinaryReader::read_u8()
{
    uint8_t res = 0;
    read(&res, 1);
    return res;
}

int8_t BinaryReader::read_s8()
{
    int8_t res = 0;
    read(&res, 1);
    return res;
}

uint16_t BinaryReader::read_u16(bool reverse)
{
    uint16_t res = 0;
    read(&res, 2);
    if (reverse)
        res = bswap_16(res);
    return res;
}

int16_t BinaryReader::read_s16(bool reverse)
{
    int16_t res = 0;
    read(&res, 2);
    if (reverse)
        res = bswap_16(res);
    return res;
}

uint32_t BinaryReader::read_u32(bool reverse)
{
    uint32_t res = 0;
    read(&res, 4);
    if (reverse)
        res = bswap_32(res);
    return res;
}

int32_t BinaryReader::read_s32(bool reverse)
{
    int32_t res = 0;
    read(&res, 4);
    if (reverse)
        res = bswap_32(res);
    return res;
}

uint64_t BinaryReader::read_u64(bool reverse)
{
    uint64_t res = 0;
    read(&res, 8);
    if (reverse)
        res = bswap_64(res);
    return res;
}

int64_t BinaryReader::read_s64(bool reverse)
{
    int64_t res = 0;
    read(&res, 8);
    if (reverse)
        res = bswap_64(res);
    return res;
}

uint64_t BinaryReader::read_un(uint16_t bytes, bool reverse)
{
    switch (bytes)
    {
        case 1:
            return read_u8();
        case 2:
            return read_u16(reverse);
        case 4:
            return read_u32(reverse);
        case 8:
            return read_u64(reverse);
        default:
            uint64_t res = 0;
            while (bytes > 0)
            {
                res = (res << 8) | read_u8();
                bytes--;
            }
            if (reverse)
            {
                res = bswap_64(res);
                res >>= ((8 - bytes) * 8);
            }
            return res;
    }
}

int64_t BinaryReader::read_sn(uint16_t bytes, bool reverse)
{
    switch (bytes)
    {
        case 1:
            return read_s8();
        case 2:
            return read_s16(reverse);
        case 4:
            return read_s32(reverse);
        case 8:
            return read_s64(reverse);
        default:
            int64_t res = 0;
            while (bytes > 0)
            {
                res = (res << 8) | read_s8();
                bytes--;
            }
            if (reverse)
            {
                res = bswap_64(res);
                res >>= ((8 - bytes) * 8);
            }
            return res;
    }
}

uint64_t BinaryReader::set_cursor(uint64_t pos)
{
    _read_pos = MIN(pos, fileSize);
    return _read_pos;
}

uint64_t BinaryReader::set_file_cursor(uint64_t abs_offset)
{
    int ret = 0;
    (void)ret;
    uint64_t act_mov = MIN(abs_offset, fileSize);
    ret              = fseek64(fp, act_mov, SEEK_SET);
    if (ret < 0)
    {
        return ret;
    }
    else
    {
        _read_pos = act_mov;
    }
    return act_mov;
}
