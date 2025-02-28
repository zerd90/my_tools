

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <string>

#include "logger.h"

using std::string;

#if defined(WIN32) || defined(_WIN32)
using std::wstring;
wstring stringToWstring(const string &orig)
{
    int               n = ::MultiByteToWideChar(CP_UTF8, 0, orig.c_str(), -1, nullptr, 0);
    wchar_t* wStr = new wchar_t[n];
    ::MultiByteToWideChar(CP_UTF8, 0, orig.c_str(), -1, wStr, n);
    wstring res(wStr);
    return res;
}
std::string stringTransToConsoleCP(const std::string &orig)
{
    // from ASCII To Unicode
    int      nlen     = MultiByteToWideChar(CP_ACP, 0, orig.c_str(), -1, NULL, NULL);
    wchar_t *pUnicode = new wchar_t[nlen];
    memset(pUnicode, 0, nlen * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, orig.c_str(), -1, (LPWSTR)pUnicode, nlen);
    // From Unicode To Console CP
    int consoleCP = GetConsoleCP();
    // printf("consoleCP=%d\n", consoleCP);
    nlen        = WideCharToMultiByte(consoleCP, 0, pUnicode, -1, NULL, 0, NULL, NULL);
    char *cConsoleCP = new char[nlen];
    WideCharToMultiByte(consoleCP, 0, pUnicode, -1, cConsoleCP, nlen, NULL, NULL);
    std::string strUTF8 = cConsoleCP;

    delete[] pUnicode;
    delete[] cConsoleCP;

    return strUTF8;
}
#else
std::string stringTransToConsoleCP(const std::string &orig)
{
    return orig;
}
#endif

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
