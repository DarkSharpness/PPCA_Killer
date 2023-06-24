#ifndef _RISC_V_CPU_H_
#define _RISC_V_CPU_H_

#include "utility.h"
#include "memchip.h"

namespace dark {

constexpr address_type memory_size = 1 << 16;


struct cpu : memory_chip <memory_size> {
    size_t __clock;
    
    /* Work in one cycle. */
    bool work() noexcept {


        ++__clock;
    }
};

}

#endif
