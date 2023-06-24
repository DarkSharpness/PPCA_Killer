#ifndef _RISC_V_MEMIO_H_
#define _RISC_V_MEMIO_H_

#include "utility.h"

namespace dark {

/* The entry of load/store station. */
struct memory_entry {
    bool ready;
    register_type reg;


};


/* Buffer for memory inout. */
struct memory_buffer {
    round_queue <memory_entry,16> __q; /* Load and store buffer. */
    int counter; /* Counter before each inout instructon. */
    void load() {
        
    }
};

}

#endif
