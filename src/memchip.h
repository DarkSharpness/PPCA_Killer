#ifndef _RISC_V_MEMCHIP_H_
#define _RISC_V_MEMCHIP_H_

#include "utility.h"

namespace dark {


template <size_t __n>
struct memory_chip {
    char data[__n];

    template <class T>
    void load(size_t __pos,T &__v) {
        __v = *reinterpret_cast <T *> (data + __pos);
    }

    template <class T>
    void store(size_t __pos,const T &__v) {
        *reinterpret_cast <T *> (data + __pos) = __v;
    }

    /* Initial program data into memory from stdin. */
    void init() {
        char buffer[16];
        address_type __a = 0;
        while(read_token(buffer)) {
            if(buffer[0] == '@') {
                __a = to_integer <address_type> (buffer + 1);
            } else {
                data[__a] = to_integer <char> (buffer);
                ++__a;
            }
       }
    }
};

}

#endif
