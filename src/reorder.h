#ifndef _RISC_V_REORDER_H_
#define _RISC_V_REORDER_H_

#include "utility.h"

namespace dark {


/* The buffer for */
struct reorder_buffer {
    struct entry {
        word_utype     info :  1;   /* Whether branch or store.  */
        word_utype     done :  1;   /* Whether command done tag. */
        word_utype     dest :  5;   /* Destination register id.  */
        word_utype   offset : 12;   /* Offset of the data.       */
        word_utype          :  0;
        word_utype        result;   /* Result of the calculation. */
        address_type        prev;   /* The return address of pc.(Branch). */
        bool is_branch() const noexcept { return info; }
    };
    static_assert(sizeof(entry) == 12);

    /* The free state flag. */
    constexpr static byte_utype free = 0;

    round_queue <entry,31> queue; /* The round queue inside. */
    bool sync_tag = false;        /* Whether to pop in sync. */

    /* Insert one command. */
    void insert(byte_utype __info,byte_utype __dest,word_utype __off,address_type __pc)
    noexcept { queue.push({__info,0,__dest,__off,0,__pc}); }

    /* Update one command from the bus. */
    void update(byte_utype __idx) {
        
    }

    /* Work in one cycle. */
    wrapper work() noexcept {
        if(queue.size() && queue.front().done) {
            auto __tmp = queue.front();
            sync_tag = true;
            if(__tmp.dest != free) return {__tmp.result,__tmp.dest};
            /* Memory related operation or branch command. */

        }
    }

    /* Return whether the queue inside is empty. */
    bool buffer_full() const noexcept { return queue.full(); }

    /**
     * @brief This fucking operation is designed to 
     * simulate real hardware, as real hardware relies
     * on the state of last cycle.
     * 
     */
    void sync() noexcept { if(sync_tag) queue.pop() , sync_tag = false; }

};


}

#endif
