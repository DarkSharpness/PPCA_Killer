#ifndef _RISC_V_REORDER_H_
#define _RISC_V_REORDER_H_

#include "utility.h"

namespace dark {


/* The buffer for */
struct reorder_buffer {
    struct entry {
        word_utype    tag : 2;   /* Whether branch or store.  */
        word_utype   done : 1;   /* Whether command done tag. */
        word_utype     result;   /* Result of the calculation. */
    }; static_assert(sizeof(entry) == 8);

    round_queue <entry,FREE> queue; /* The round queue inside. */
    bool sync_tag = false;          /* Whether to pop in sync. */

    /**
     * @brief Work in one cycle. 
     * 
     * @return Commit message.
     */
    wrapper work() noexcept {
        if(queue.size() && queue.front().done) {
            auto __tmp = queue.front();
            sync_tag   = true;
            return {__tmp.result,__tmp.tag << 5 | queue.head};
        } else return {};
    }

    /* Return whether the queue inside is empty. */
    bool buffer_full() const noexcept { return queue.full(); }

    /* Update one command from the bus. */
    void update(const return_list &__list) noexcept {
        for(auto &&iter : __list) {
            queue[iter.idx].done    = true;
            queue[iter.idx].result ^= iter.val;
        }
    }

    /* Initial result as 0.  */
    word_utype init_result(word_utype __tag,address_type __pc) {
        if      (__tag == JUMP_TAG) return __pc |  1;
        else if (__tag == STAY_TAG) return __pc & ~1;
        else                        return 0;
    }

    /**
     * @brief Insert one command into reorder buffer.
     * 
     * @attention Use it in the end of a cycle.
     */
    void insert(word_utype __tag,address_type __pc) noexcept {
        queue.push({__tag,
                    __tag == STORE_TAG, /* Store is naturally done. */
                    init_result(__tag,__pc)  /* If jump,tag == 1 */
                });
    }

    /**
     * @brief This fucking operation is designed to 
     * simulate real hardware, as real hardware relies
     * on the state of last cycle.
     * 
     * @attention This function should only be operated
     * in the end of the cycle.
     */
    void sync() noexcept { if(sync_tag) queue.pop() , sync_tag = false; }

};


}

#endif
