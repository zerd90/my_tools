#ifndef Z_LOGGER_H
#define Z_LOGGER_H

#define MAX_CMD 4096

#include <string.h>
#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <iostream>

#ifdef __linux

#elif defined(WIN32) || defined(_WIN32)
    #include <windows.h>
    #ifdef _MSC_VER
        #define __PRETTY_FUNCTION__ __FUNCSIG__
    #endif
#endif

enum LOG_LEVEL
{
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERR  = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DBG  = 4,
    LOG_LEVEL_ALL  = LOG_LEVEL_DBG,
    LOG_LEVEL_BUTT,
};

std::wstring stringToWstring(const std::string &orig);

std::string wstringToString(const std::wstring &wstr);

namespace Log
{
    void      set_log_level(LOG_LEVEL level);
    LOG_LEVEL get_log_level();

    struct ArgBase
    {
        virtual std::string get_ss(std::string var_fmt = std::string()) = 0;
        virtual ~ArgBase() {}
    };

    template <typename T, typename U>
    struct decay_equiv : std::is_same<typename std::decay<T>::type, U>::type
    {
    };

    template <typename T>
    struct ArgChar : ArgBase
    {
        T arg;

        explicit ArgChar(T val) : arg(val)
        {
            static_assert(decay_equiv<T, char>::value || decay_equiv<T, unsigned char>::value, "type error");
        }
        ~ArgChar() {}

        std::string get_ss(std::string var_fmt = std::string()) override
        {
            std::ostringstream ss;
            if (!var_fmt.empty())
            {
                char buf[64];
                snprintf(buf, sizeof(buf), var_fmt.c_str(), arg);
                ss << buf;
            }
            else
            {
                ss << (int)arg;
            }

            return ss.str();
        }
    };

    template <typename T>
    struct ArgWString : ArgBase
    {
        T arg;

        explicit ArgWString(T val) : arg(val) { static_assert(decay_equiv<T, std::wstring>::value, "type error"); }
        ~ArgWString() {}

        std::string get_ss(std::string var_fmt = std::string()) override { return wstringToString(arg); }
    };

    template <typename T>
    struct ArgBaseType : ArgBase
    {
        T arg;

        explicit ArgBaseType(T val) : arg(val) {}

        ~ArgBaseType() {}

        std::string get_ss(std::string var_fmt = std::string()) override
        {
            std::ostringstream ss;
            if (!var_fmt.empty())
            {
                char buf[64];
                snprintf(buf, sizeof(buf), var_fmt.c_str(), arg);
                ss << buf;
            }
            else
            {
                ss << arg;
            }

            return ss.str();
        }
    };

    template <typename T>
    struct Arg : ArgBase
    {
        T arg;

        explicit Arg(T val) : arg(val) {}

        ~Arg() {}

        std::string get_ss(std::string var_fmt = std::string()) override
        {
            std::ostringstream ss;
            ss << arg;
            return ss.str();
        }
    };

    struct Arguments
    {
        std::vector<std::shared_ptr<ArgBase>> args;

        ~Arguments() {}

        int transfer() { return 0; }
        template <typename T, typename... Args>
        int transfer(T arg1, Args &&...args)
        {
            std::shared_ptr<ArgBase> arg;
            if constexpr (decay_equiv<T, char>::value || decay_equiv<T, unsigned char>::value)
            {
                arg = std::make_shared<ArgChar<T>>(arg1);
            }
            else if constexpr (decay_equiv<T, std::wstring>::value)
            {
                arg = std::make_shared<ArgWString<T>>(arg1);
            }
            else if constexpr (decay_equiv<T, wchar_t *>::value)
            {
                arg = std::make_shared<ArgWString<std::wstring>>(std::wstring(arg1));
            }
            else if constexpr (decay_equiv<T, short>::value || decay_equiv<T, unsigned short>::value
                               || decay_equiv<T, int>::value || decay_equiv<T, unsigned int>::value
                               || decay_equiv<T, long>::value || decay_equiv<T, unsigned long>::value
                               || decay_equiv<T, long long>::value || decay_equiv<T, unsigned long long>::value
                               || decay_equiv<T, float>::value || decay_equiv<T, double>::value
                               || std::is_pointer<T>::value || std::is_enum<T>::value)
            {
                arg = std::make_shared<ArgBaseType<T>>(arg1);
            }
            else
            {
                arg = std::make_shared<Arg<T>>(arg1);
            }
            if (arg == nullptr)
                return -1;

            this->args.push_back(arg);
            return transfer(args...);
        }

        std::string get_ss(unsigned int idx, std::string var_fmt = std::string())
        {
            if (idx >= args.size())
                return "";
            return args[idx]->get_ss(var_fmt);
        }
    };

    template <typename... Args>
    std::string format(const char *fmt, Args &&...args)
    {
        std::ostringstream ss;
        Arguments          to_args;
        std::string        fmt_str(fmt);

        int         ret     = 0;
        const char *end     = fmt + strlen(fmt);
        const char *pre_pos = fmt;
        uint64_t    pre_idx = 0;
        int         arg_idx = 0;
        const char *c       = fmt;

        ret = to_args.transfer(args...);
        if (ret < 0)
            return std::string();

        while (*c != '\0')
        {
            if ('\\' == *c)
            {
                if ('{' == *(c + 1))
                {
                    ss << fmt_str.substr(pre_idx, c - pre_pos);
                    c++;
                    pre_pos = c;
                    pre_idx = c - fmt;
                }
                else
                {
                    c++;
                }
            }
            else if ('{' == *c)
            {
                uint64_t r_pos = fmt_str.find('}', c - fmt);
                if (r_pos == std::string::npos)
                    break;
                else
                {
                    ss << fmt_str.substr(pre_idx, c - pre_pos);
                    if (r_pos - 1 > (uint64_t)(c - fmt))
                    {
                        std::string var_fmt("%");
                        var_fmt.append(fmt_str.substr(c - fmt + 1, r_pos - 1 + fmt - c));
                        ss << to_args.get_ss(arg_idx++, var_fmt);
                    }
                    else
                    {
                        ss << to_args.get_ss(arg_idx++);
                    }

                    c       = fmt + r_pos + 1;
                    pre_pos = c;
                    pre_idx = r_pos + 1;
                }
            }
            else
            {
                c++;
            }
        }

        if (pre_pos != end)
        {
            ss << fmt_str.substr(pre_idx, end - pre_pos);
        }

        return ss.str();
    }
    template <typename... Args>
    std::wstring wformat(const char *fmt, Args &&...args)
    {
        return stringToWstring(format(fmt, args...));
    }

#define FORMAT_CSTR(fmt, ...) Log::format(fmt, ##__VA_ARGS__).c_str()

#if defined(WIN32) || defined(_WIN32)
    #define color_output(l, wAttributes)                                                                         \
        template <typename... Args>                                                                              \
        void l(const char *fmt, Args &&...args)                                                                  \
        {                                                                                                        \
            HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);                                                    \
            SetConsoleTextAttribute(hStdout, wAttributes);                                                       \
            fprintf(stderr, "%s", format(fmt, args...).c_str());                                                 \
            SetConsoleTextAttribute(hStdout,                                                                     \
                                    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY); \
        }

    color_output(blue, FOREGROUND_BLUE);
    color_output(green, FOREGROUND_GREEN);
    color_output(red, FOREGROUND_RED);
    color_output(cyan, FOREGROUND_BLUE | FOREGROUND_GREEN);
    color_output(purple, FOREGROUND_BLUE | FOREGROUND_RED);
    color_output(brown, FOREGROUND_GREEN | FOREGROUND_RED);
    color_output(dark_gray, FOREGROUND_INTENSITY);

    color_output(light_blue, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    color_output(light_green, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    color_output(light_red, FOREGROUND_RED | FOREGROUND_INTENSITY);
    color_output(light_cyan, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    color_output(yellow, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
    color_output(light_purple, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
    color_output(light_gray, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    color_output(black, 0);
    color_output(white, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

#else
    #define LOG_NONE "\033[0m"
    #define LOG_BLACK "\033[0;30m"
    #define LOG_DARK_GRAY "\033[1;30m"
    #define LOG_BLUE "\033[0;34m"
    #define LOG_LIGHT_BLUE "\033[1;34m"
    #define LOG_GREEN "\033[0;32m"
    #define LOG_LIGHT_GREEN "\033[1;32m"
    #define LOG_CYAN "\033[0;36m"
    #define LOG_LIGHT_CYAN "\033[1;36m"
    #define LOG_RED "\033[0;31m"
    #define LOG_LIGHT_RED "\033[1;31m"
    #define LOG_PURPLE "\033[0;35m"
    #define LOG_LIGHT_PURPLE "\033[1;35m"
    #define LOG_BROWN "\033[0;33m"
    #define LOG_YELLOW "\033[1;33m"
    #define LOG_LIGHT_GRAY "\033[0;37m"
    #define LOG_WHITE "\033[1;37m"

    #define color_output(l, u)                                   \
        template <typename... Args>                              \
        void l(const char *fmt, Args &&...args)                  \
        {                                                        \
            fprintf(stderr, LOG_##u);                            \
            fprintf(stderr, "%s", format(fmt, args...).c_str()); \
            fprintf(stderr, LOG_NONE);                           \
        }

    color_output(blue, BLUE);
    color_output(green, GREEN);
    color_output(red, RED);
    color_output(cyan, CYAN);
    color_output(purple, PURPLE);
    color_output(brown, BROWN);
    color_output(dark_gray, DARK_GRAY);

    color_output(light_blue, LIGHT_BLUE);
    color_output(light_green, LIGHT_GREEN);
    color_output(light_cyan, LIGHT_CYAN);
    color_output(light_red, LIGHT_RED);
    color_output(yellow, YELLOW);
    color_output(light_purple, LIGHT_PURPLE);
    color_output(light_gray, LIGHT_GRAY);

    color_output(black, BLACK);
    color_output(white, WHITE);

#endif

    template <typename... Args>
    void print(const char *fmt, Args &&...args)
    {
        fprintf(stderr, "%s", format(fmt, args...).c_str());
    }
}; // namespace Log

static inline const char *get_file_name(const char *fn)
{
    uint64_t len = strlen(fn);
    for (uint64_t i = len - 1; i >= 0; i--)
    {
        if (fn[i] == '/' || fn[i] == '\\')
        {
            return &fn[i + 1];
        }
    }
    return fn;
}

static inline std::string get_file_name(std::string &fn)
{
    uint64_t len  = fn.length();
    size_t   pos1 = fn.rfind('/');
    size_t   pos2 = fn.rfind('\\');
    if (pos1 == std::string::npos && pos2 == std::string::npos)
    {
        return fn;
    }
    else if (pos1 != std::string::npos)
    {
        return fn.substr(pos1 + 1, len - pos1);
    }
    else if (pos2 != std::string::npos)
    {
        return fn.substr(pos2 + 1, len - pos2);
    }
    return fn;
}

std::string getBaseName(std::string &path);
std::string getBaseName(std::string &&path);

static inline std::string _CutParenthesesNTail(const char *s)
{
    std::string prettyFunction(s);
    uint64_t    bracket = prettyFunction.rfind("(");
    uint64_t    space;
    space = prettyFunction.rfind(" ", bracket) + 1;
    return prettyFunction.substr(space, bracket - space);
}
#define __CLASS_FUNCTION__ _CutParenthesesNTail(__PRETTY_FUNCTION__)

#define LOG_PREFIX(color) Log::color("[{}:{} ({})]", get_file_name(__FILE__), __LINE__, __CLASS_FUNCTION__);
#define ZM_LOG(color, fmt, ...)         \
    do                                  \
    {                                   \
        LOG_PREFIX(color);              \
        Log::print(fmt, ##__VA_ARGS__); \
    } while (0)

#define ZM_INFO(fmt, ...) ZM_LOG(green, fmt, ##__VA_ARGS__)
#define ZM_ERR(fmt, ...) ZM_LOG(red, fmt, ##__VA_ARGS__)
#define ZM_WARN(fmt, ...) ZM_LOG(yellow, fmt, ##__VA_ARGS__)
#define ZM_DBG(fmt, ...) ZM_LOG(blue, fmt, ##__VA_ARGS__)

#define Z_ERR(fmt, ...)                            \
    do                                             \
    {                                              \
        if (Log::get_log_level() >= LOG_LEVEL_ERR) \
        {                                          \
            ZM_LOG(red, fmt, ##__VA_ARGS__);       \
        }                                          \
    } while (0)

#define Z_WARN(fmt, ...)                            \
    do                                              \
    {                                               \
        if (Log::get_log_level() >= LOG_LEVEL_WARN) \
        {                                           \
            ZM_LOG(yellow, fmt, ##__VA_ARGS__);     \
        }                                           \
    } while (0)

#define Z_INFO(fmt, ...)                            \
    do                                              \
    {                                               \
        if (Log::get_log_level() >= LOG_LEVEL_INFO) \
        {                                           \
            ZM_LOG(green, fmt, ##__VA_ARGS__);      \
        }                                           \
    } while (0)

#define Z_DBG(fmt, ...)                            \
    do                                             \
    {                                              \
        if (Log::get_log_level() >= LOG_LEVEL_DBG) \
        {                                          \
            ZM_LOG(blue, fmt, ##__VA_ARGS__);      \
        }                                          \
    } while (0)

#define Z_LOG(fmt, ...) Log::print(fmt, ##__VA_ARGS__)

void str_insert(char *str, uint64_t size, char c, int pos);

std::string get_secure_path(std::string src);

#endif
