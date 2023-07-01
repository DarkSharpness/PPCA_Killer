#ifndef _RISC_V_REORDER_H_
#define _RISC_V_REORDER_H_

#include "utility.h"

namespace dark {


/* The buffer for */
struct reorder_buffer {
    struct entry {
        word_utype     result;  /* Result of the calculation. */
        word_utype   done : 1;  /* Whether command done tag.  */
        word_utype    tag : 2;  /* Tag of type of command.    */
        word_utype   dest : 5;  /* Destination in register file. */
    }; static_assert(sizeof(entry) == 8);

    round_queue <entry,FREE> queue; /* The round queue inside. */
    bool sync_tag = false;          /* The sync tag.           */

    /**
     * @brief Work in one cycle. 
     * 
     * @return Commit message. With result + tag + destination register.
     */
    wrapper work() noexcept {
        if(queue.size() && queue.front().done) {
            sync_tag = true;
            auto __tmp = queue.front();
            return {__tmp.result,(address_type)(__tmp.tag << 5) | __tmp.dest};
        } else return {0,0};
    }

    /* A wire of the head index of buffer. */
    uint32_t buffer_head() const noexcept { return queue.head; }

    /* A wire of the next available address in queue. */
    uint32_t buffer_tail() const noexcept { return queue.tail(); }

    /* Whether the buffer is full. */
    bool is_full() const noexcept { return queue.full();  }

    /* A wire of whether the RoB is empty. */
    bool empty()   const noexcept { return queue.empty(); }

    /* Update one command from the bus. */
    void update(const return_list &__list) noexcept {
        for(auto &&iter : __list) {
            queue[iter.index()].done    = true;
            queue[iter.index()].result |= iter.value();
            /* 
                This is a smart trick for prediction case.
                In BRANCH,the result will be stored in the last bit.
                Since pc is stored into result with last bit as 0,
                we can perform OR operation to store results.
             */
        }
    }

    /**
     * @brief Insert one command into reorder buffer.
     * 
     * @param __arg If BRANCH, __arg = pc (loweset bit = predicition)
     * If STORE, __arg = data_pack (parsed command)
     * If JAL/AUIPC/LUI , __arg = new result (And naturally, done = true) 
     * If other cases, __arg = 0.
     * @attention Use it in the end of a cycle.
     */
    void insert(word_utype  __arg,word_utype  __tag,
                word_utype __dest,word_utype __done)
    noexcept { queue.push({__arg,__done,__tag,__dest}); }

    /* Clear the pipeline when prediction fails. */
    void clear_pipeline() noexcept { queue.clear(); sync_tag = false; }

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
