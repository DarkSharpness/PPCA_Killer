#ifndef _RISC_V_MEMIO_H_
#define _RISC_V_MEMIO_H_

#include "utility.h"
#include "memchip.h"
#include "instruction.h"

namespace dark {


constexpr address_type memory_size = 1 << 21;

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
        auto size() const noexcept { return code & 0b011; }

        /* Set this command as done. */
        void set_done()       noexcept { code |= 0b011; }
        /* Whether this command is done. */
        bool is_done()  const noexcept { return size() == 0b011; }
        /* Whether this command is available to operate. */
        bool is_ready() const noexcept { return   idx1 == FREE;  }

        /* Return the real address. */
        address_type address() const noexcept
        { return source1 + offset; }
    }; static_assert(sizeof(entry) == 12);


    round_queue <entry,32> loader;  /* Load  buffer.   */
    entry current;                  /* Current  entry. */

    address_type pc =   0 ;     /* PC pointer. */

    bool   load_tag = false;    /* Whether current is load operation. */
    byte_utype last = FREE;     /* Last store RoB index in RoB. */
    byte_utype index;           /* Index of current opeartion in loader queue. */
    byte_stype  cc =  -1 ;      /* Stupid counter...... */
    /**
     * @brief Inner method of fetching a command.
     * Note that this command is only used in C++
     * simulation, as decode center should directly
     * fetch command from memory chip.
     * 
     * @param __pc The real PC value.
     * @return command_type 
     */
    void fetch(command_type &__cmd) noexcept { memory_chip::load(pc,__cmd,4); }

    /**
     * @brief Work for one cycle. 
     * 
     * @return Whether load information is done.
     */
    return_list work() noexcept {
        if(cc == -1 || cc-- || !load_tag) return {};

        /* Now the loading work is done and must be commited at once. */
        memory_chip::load(current.address(),
                          current.source2,
                          1 << current.size());

        /* Sign extension or not. */
        if(current.sign()) {
            if(current.size() == 0)
                current.source2 = (int8_t)  current.source2;
            if(current.size() == 1)
                current.source2 = (int16_t) current.source2;
        } loader[index].set_done();

        return {{current.source2,current.dest}};
    }

    /* Clear the pipeline when prediction fails. */
    void clear_pipeline() noexcept
    { loader.clear() , last = FREE, load_tag = false , cc = -1; }

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
                        wrapper    __data) noexcept
    { loader.push({__code,last,__dest,__data.index(),__offset,__data.value(),0}); }

    /* Special insert for store command. */
    void insert_store(word_utype __dest) { last = __dest; } 

    /**
     * @brief Store data when store command is commited.
     * 
     * @attention Use it in the end of a cycle.
     */
    void store (word_utype  __code,
                word_utype  __dest,
                address_type __addr,
                address_type __reg2) noexcept {
        if(last == __dest) last = FREE;

        load_tag = false , cc += 3;  /* Store time. */
        memory_chip::store(__addr,__reg2,1 << (__code & 0b11));

        /* Update the prev pointers in the queue. */
        int head = loader.head;
        int size = loader.dist;
        while(size-- && loader[head].prev == __dest) {
            loader[head].prev = FREE;
            if(++head == loader.length()) head = 0;
        }
    }

    /**
     * @brief Update the dependency from RoB commit.
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
            } if(++head == loader.length()) head = 0;
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
        /* Is calculating. */
        if(cc != -1) return;

        /* Pop out all useless elements first. */
        while(loader.size() && loader.front().is_done()) loader.pop();

        /* Find the first ready and undone , then work on it. */
        int head = loader.head;
        int size = loader.dist;
        while(size-- && loader[head].prev == FREE) {
            auto &__c = loader[head];
            if(__c.is_ready() && !__c.is_done()) {
                load_tag = true , cc += 3;
                current  = __c;
                index    = head; return;
            } if(++head == loader.length()) head = 0;
        }
    }

    /* Capacity of the loader. */
    constexpr int capacity() const noexcept { return loader.length(); }

};

}

#endif
