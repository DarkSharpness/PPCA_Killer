#ifndef _RISC_V_MEMIO_H_
#define _RISC_V_MEMIO_H_

#include "utility.h"
#include "memchip.h"
#include "instruction.h"

namespace dark {


constexpr address_type memory_size = 1 << 20;

/**
 * @brief A buffered fixed-sized memory_chip.
 * 
 */
struct memory : memory_chip <memory_size> {
    /* Entry of one memory buffer. */
    struct entry {
        word_utype        :  2;
        word_utype sign   :  1; /* Load signed or unsigned. */
        word_utype prev   :  5; /* Last store operation. */
        word_utype dest   :  5; /* Index in the reorder buffer. */
        word_utype size   :  2; /* The bit_length of data.  */
        word_utype idx1   :  5; /* Index of constraint1 in reorder. */
        word_stype offset : 12; /* Offset of address (with additional sign bit) */

        register_type  source1;  /* The source register value.           */
        register_type  source2;  /* Result of the calculation or source. */

        /* Set this command as done. */
        void set_done() noexcept       { size = 3; }
        /* Whether this command is done. */
        bool is_done() const noexcept  { return size == 3; }
        /* Whether this command is available to operate. */
        bool is_ready() const noexcept { return idx1 == FREE; }
        /* Return the real address. */
        address_type address() const noexcept
        { return source1 + offset; }
    }; static_assert(sizeof(entry) == 12);


    round_queue <entry,32> loader;  /* Load  buffer. */
    entry current;                  /* Current  entry. */
    
    address_type pc =   0  ;    /* PC pointer. */
    bool load_tag   = false;    /* Whether current is load operation. */
    byte_stype cc   =  -1  ;    /* Stupid counter...... */
    byte_utype last = FREE ;     /* Lastest store information. */
    byte_stype index;           /* Index of current opeartion in queue. */

    /**
     * @brief Inner method of fetching a command.
     * Note that this command is only used in C++
     * simulation, as decode center should directly
     * fetch command from memory chip.
     * 
     * @param __pc The real PC value.
     * @return command_type 
     */
    command_type fetch(command_type __cmd)
    noexcept { memory_chip::load(pc,__cmd,4); }

    /* Inner handle. */
    void work_counter() noexcept {
        if(~cc || cc--) return;
        /* Now cc = -1. */
        if(load_tag) {
            memory_chip::load(current.address(),
                              current.source2,
                              1 << current.size);
            if(current.sign) {
                if(current.size == 0)
                    current.source2 = (int8_t)  current.source2;
                if(current.size == 1)
                    current.source2 = (int16_t) (current.source2);
            } current.set_done();
            loader[index] = current;
        } else { /* Load into memory. */
            memory_chip::store(current.address(),
                               current.source2,
                               1 << current.size);
            int head = loader.head;
            int size = loader.dist;
            while(size-- && loader[head].prev == current.dest) {
                loader[head].prev = FREE;
                if(++head == loader.length()) head = 0;
            } if(last == current.dest) last = FREE;
        } /* Set done and change the index. */
    }

    /* Inner handle. */
    void clear_pipeline() noexcept
    { loader.clear() , load_tag  = false , cc = -1 , last = FREE; }

    /**
     * @brief Work for one cycle. 
     * 
     * @return Whether load information is done.
     */
    wrapper work() noexcept {
        /* Empty and nothing doing. */
        if(loader.empty()) return {};
        /* Counter should decrease in this cycle. */
        work_counter();
        /* If head is done,then we should push now. */
        auto head = loader.front();
        if(!head.is_done()) return {};
        else return {head.source2,head.dest};
    }

    /* A wire indicating whether the loader is full. */
    bool is_full() const noexcept { return loader.full(); }

    /**
     * @brief Insert a command to the queue.
     * 
     * @attention Use it in the end of a cycle.
     */
    inline void insert (word_utype __sign,
                        word_utype __dest,
                        word_utype __length,
                        word_stype __offset,
                        wrapper    __reg1) noexcept {
        loader.push({
                    __sign,
                    last,
                    __dest,
                    __length,
                    __reg1.idx,
                    __offset,
                    __reg1.val,
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
    void store (word_utype  __dest  ,
                word_utype  __length,
                word_stype  __offset,
                register_type __reg1,
                register_type __reg2) noexcept {
        load_tag = false , cc = 2;
        current.dest    = __dest;
        current.size    = __length;
        current.offset  = 0;
        current.source1 = __reg1;
        current.source2 = __reg2;
    }

    /**
     * @brief Update the dependency from RoB.
     * 
     * @attention Use it after insertion.
     */
    void update(wrapper __data) noexcept {
        int head = loader.head;
        int size = loader.dist;
        if(!__data.is_file()) {
            if(__data.is_jump_WA()) clear_pipeline();
            return; /* BRANCH || JALR || STORE. */
        } while(size--) {
            auto &__c = loader[head];
            if(__c.idx1 == __data.idx) {
                __c.idx1    = FREE;
                __c.source1 = __data.val;
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
    void sync() {
        /* Is calculating || Nothing can be done. */
        if(cc != -1 || loader.empty()) return;

        /* Pop out the first load element if done. */
        if(loader.front().is_done()) loader.pop();

        /* Tries to load a new element. */
        int head = loader.head;
        int size = loader.dist;
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
