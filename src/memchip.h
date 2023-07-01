#ifndef _RISC_V_MEMCHIP_H_
#define _RISC_V_MEMCHIP_H_

#include "utility.h"

namespace dark {


template <size_t __n>
struct memory_chip {
    char data[__n]; /* The real internal data. */

    /* Load a trivial type from memory. */
    template <class T>
    void load(address_type __pos,T &__v,size_t __m) /*noexcept*/ { 
        // if(__pos + __m > __n) return;
        memcpy(&__v,data + __pos,__m);
    }

    /* Store a trivial type into memory. */
    template <class T>
    void store(address_type __pos,const T &__v,size_t __m) /*noexcept*/ {
        // if(__pos + __m > __n) return;
        memcpy(data + __pos,&__v,__m);
    }

    /* Initial program data into memory from stdin. */
    void init() /*noexcept*/ {
        char buffer[32];
        address_type __a = 0;
        while(read_token(buffer)) {
            if(buffer[0] == '@') {
                __a = hex_to_integer <address_type> (buffer + 1);
            } else {
                data[__a++] = char_map(buffer[0]) << 4 |
                              char_map(buffer[1]);
            }
       }
    }
};

}

#endif
