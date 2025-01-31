#ifndef Z_BASIC_TOOLS_H
#define Z_BASIC_TOOLS_H

#undef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#undef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#undef ABS
#define ABS(a) ((a) > 0 ? (a) : (-(a)))

#ifndef IN_RANGE
    #define IN_RANGE(a, b, c) (((b) >= (a)) && ((b) <= (c)))
#endif

#ifndef ROUND
    #define ROUND(a, b, c) (MIN(MAX((a), (b)), (c)))
#endif

#endif