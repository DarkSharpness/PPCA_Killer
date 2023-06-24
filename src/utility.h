#ifndef _RISC_V_UTILITY_H_
#define _RISC_V_UTILITY_H_

#include "round_queue.h"
#include "register.h"

#include <stdint.h>
#include <locale>

namespace dark {

using word_type = int32_t;
using half_type = int16_t;
using byte_type = int8_t;

using uword_type = uint32_t;
using uhalf_type = uint16_t;
using ubyte_type = uint8_t;

using address_type = uint32_t;
using command_type = uint32_t;

using register_type = _register;

/* Judge whether given char is a visible char */
bool is_visible_char(int __c) noexcept
{ return __c > 126 || __c < 33; }

/* Read a token. Return whether EOF isn't reached. */
bool read_token(char *__str) noexcept {
    int __c;
    while(is_visible_char(__c = getchar())) {
        *(__str++) = __c;
    } *__str = 0; 
    return __c != EOF;
}

/* Map a char into a hex number. */
int char_map(char x) noexcept { return isdigit(x) ? x - '0' : x - 'A' + 10; }

/* Turn a hex number string into a trivial number type. */
template <class T>
T to_integer(char *__str) noexcept {
    T __v = 0;
    while(*__str) __v = __v << 4 | char_map(*__str++);
    return __v;
}

}

#endif
