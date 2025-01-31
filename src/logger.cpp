

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <string>

#include "logger.h"

using std::string;

namespace Log
{
    LOG_LEVEL log_level = LOG_LEVEL_ERR;
    void      set_log_level(LOG_LEVEL level)
    {
        if (level < LOG_LEVEL_ERR || level >= LOG_LEVEL_BUTT)
            return;
        log_level = level;
    }
    LOG_LEVEL get_log_level()
    {
        return log_level;
    }
} // namespace Log

std::string getBaseName(std::string &path)
{
    size_t lastSlash = path.rfind('\\');
    if (path.npos == lastSlash)
        lastSlash = path.rfind('/');
    if (path.npos == lastSlash)
        lastSlash = 0;
    else
        lastSlash++;

    return path.substr(lastSlash, path.rfind('.') - lastSlash);
}

std::string getBaseName(std::string &&path)
{
    size_t lastSlash = path.rfind('\\');
    if (path.npos == lastSlash)
        lastSlash = path.rfind('/');
    if (path.npos == lastSlash)
        lastSlash = 0;
    else
        lastSlash++;

    return path.substr(lastSlash, path.rfind('.') - lastSlash);
}

void str_insert(char *str, uint64_t max_size, char c, int pos)
{
    uint64_t str_size = strlen(str);
    if (str_size + 2 > max_size)
    {
        Z_ERR("size too small %d, %d\n", str_size, max_size);
        return;
    }
    char *dst = NULL;

    dst = (char *)calloc(str_size + 2, 1);
    if (!dst)
    {
        Z_ERR("alloc fail\n");
        return;
    }

    if (pos > 0)
        memcpy(dst, str, pos);
    dst[pos] = c;
    memcpy(dst + pos + 1, str + pos, str_size - pos + 1);

    memcpy(str, dst, str_size + 2);

    free(dst);
}
