#ifndef _RISC_V_CPU_H_
#define _RISC_V_CPU_H_

#include "utility.h"
#include "memchip.h"
#include "register.h"

namespace dark {

/**
 * @brief A simple CPU simulator.
 * 
 */
struct cpu : register_file {
    size_t clock; /* Internal clock. */

    /* Work in one cycle. */
    bool work() noexcept {


        ++clock;
    }
};

}

#endif
