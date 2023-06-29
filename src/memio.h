#ifndef _RISC_V_MEMIO_H_
#define _RISC_V_MEMIO_H_

#include "utility.h"
#include "memchip.h"
#include "instruction.h"

namespace dark {


constexpr address_type memory_size = 1 << 20;

/**
 * @brief A buffered fixed-sized memory chip.
 * 
 */
struct memory : memory_chip <memory_size> {
    /* Entry of one memory buffer. */
    struct entry {
        word_utype code   :  3; /* The code */
        word_utype prev   :  5; /* Last store operation. */
        word_utype        :  2;
        word_utype dest   :  5; /* Index in the reorder buffer. */
        word_utype idx1   :  5; /* Index of constraint1 in reorder. */
        word_stype offset : 12; /* Offset of address (with additional sign bit) */

        register_type  source1;  /* The source register value.           */
        register_type  source2;  /* Result of the calculation or source. */


        /* Load signed or unsigned. */
        bool sign() const noexcept { return code & 0b100; }
        /* The bit_length of data.  */
        bool size() const noexcept { return code & 0b011; }

        /* Set this command as done. */
        void set_done() noexcept       { code |= 0b011; }
        /* Whether this command is done. */
        bool is_done()  const noexcept { size() == 0b011; }
        /* Whether this command is available to operate. */
        bool is_ready() const noexcept { return idx1 == FREE; }

        /* Return the real address. */
        address_type address() const noexcept
        { return source1 + offset; }
    }; static_assert(sizeof(entry) == 12);


    round_queue <entry,32> loader;  /* Load  buffer. */
    entry current;                  /* Current  entry. */
    
    address_type pc =   0  ;    /* PC pointer. */

    byte_stype cc   =  -1  ;    /* Stupid counter...... */
    byte_utype last = FREE ;    /* Lastest store information. */
    byte_utype index;   /* Index of current opeartion in queue. */
    bool    load_tag;   /* Whether current is load operation. */


    /**
     * @brief Inner method of fetching a command.
     * Note that this command is only used in C++
     * simulation, as decode center should directly
     * fetch command from memory chip.
     * 
     * @param __pc The real PC value.
     * @return command_type 
     */
    void fetch(command_type &__cmd)
    noexcept { memory_chip::load(pc,__cmd,4); }

    /* Inner handle. */
    return_list work_counter() noexcept {
        if(~cc || cc--) return {};

        /* Now the work is done and must be commited at once. */
        if(load_tag) { /* Load from memory */
            memory_chip::load(current.address(),
                              current.source2,
                              1 << current.size());
            if(current.sign()) {
                if(current.size() == 0)
                    current.source2 = (int8_t)  current.source2;
                if(current.size() == 1)
                    current.source2 = (int16_t) (current.source2);
            }
            loader[index].set_done();
            return {{current.source2,current.dest}};
        } else { /* Store into memory. */
            memory_chip::store(current.address(),
                               current.source2,
                               1 << current.size());

            int head = loader.head;
            int size = loader.dist;
            /* Update the prev pointers in the queue. */
            while(size-- && loader[head].prev == current.dest) {
                loader[head].prev = FREE;
                if(++head == loader.length()) head = 0;
            } if(last == current.dest) last = FREE;
            return {};
        }
    }

    /**
     * @brief Work for one cycle. 
     * 
     * @return Whether load information is done.
     */
    return_list work() noexcept {
        /* Empty and nothing doing. */
        if(loader.empty()) return {};
        /* Counter should decrease in this cycle. */
        return work_counter();
    }

    /* Clear the pipeline. */
    void clear_pipeline() noexcept
    { loader.clear() , cc = -1 , last = FREE; }

    /* A wire indicating whether the loader is full. */
    bool is_full() const noexcept { return loader.full(); }

    /**
     * @brief Insert a command to the queue.
     * 
     * @attention Use it in the end of a cycle.
     */
    inline void insert (word_utype __code,
                        word_utype __dest,
                        word_stype __offset,
                        wrapper    __reg1) noexcept {
        loader.push({
                    __code,
                    last,
                    __dest,
                    __reg1.index(),
                    __offset,
                    __reg1.value(),
                    0
                });
    }

    /* Special insert for store command. */
    void insert_store(word_utype __dest) { last = __dest; } 

    /**
     * @brief Store data when store command is commited.
     * 
     * @attention Use it in the end of a cycle.
     */
    void store (word_utype  __dest,
                word_utype  __code,
                register_type __reg1,
                register_type __reg2) noexcept {
        load_tag = false , cc = 2;
        current.dest    = __dest;
        current.code    = __code;
        current.source1 = __reg1;
        current.source2 = __reg2;
    }

    /**
     * @brief Update the dependency from RoB.
     * 
     * @param wrapper Register file modification.
     * @attention Use it after insertion.
     */
    void update(wrapper __data) noexcept {
        int head = loader.head;
        int size = loader.dist;
        while(size--) {
            auto &__c = loader[head];
            if(__c.idx1 == __data.index()) {
                __c.idx1    = FREE;
                __c.source1 = __data.value();
            }
        }
    }

    /**
     * @brief This fucking operation is designed to 
     * simulate real hardware, as real hardware relies
     * on the state of last cycle.
     * 
     * @attention This function should only be operated
     * in the end of a cycle.
     */
    void sync() noexcept {
        /* Is calculating || Nothing can be done. */
        if(cc != -1 || loader.empty()) return;

        /* Tries to load a new element. */
        while(loader.size() && loader.front().is_done()) loader.pop();

        int head = loader.head;
        int size = loader.dist;
        /* Find the first ready and undone , then work on it. */
        while(size-- && loader[head].prev == FREE) {
            if(loader[head].is_ready() && !loader[head].is_done()) {
                load_tag = true, cc = 2;
                current  = loader[head];
                index    = head;
                return;
            } if(++head == loader.length()) head = 0;
        }
    }

};

}

#endif
