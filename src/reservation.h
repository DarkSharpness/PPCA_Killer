#ifndef _RISC_V_RESERVATION_H_
#define _RISC_V_RESERVATION_H_

#include "utility.h"
#include "alu.h"

namespace dark {

/* Station for instructions. */
struct reservation_station {
    struct entry {
        ALU_code    op  : 7;    /* Operator bit.     */
        byte_utype      : 1;
        byte_utype idx1 : 5;    /* Index of constraint 1 reorder. */
        byte_utype idx2 : 5;    /* Index of constraint 2 reorder.  */
        byte_utype dest : 5;    /* Index of destination in reorder buffer. */
        byte_utype      : 1;
        register_type  src1;    /* Source value 1. */
        register_type  src2;    /* Source value 2.  */
        register_type result;   /* The result of reservation station. */

        /* Whether this entry is available to be executed. */
        bool is_ready() const noexcept { return idx1 == FREE && idx2 == FREE; }
    }; static_assert(sizeof(entry) == 16);


    [[no_unique_address]]
    ALU_type unit[4];   /*    ALUs.     */
    entry  array[32];   /* Entry array. */

    std::bitset <32> array_state{ 0U};  /* Data in array.  */
    std::bitset <32> array_syncs{-1U};  /* Array's sync data. */

    /* A wire indicating whether the arithmetic station is full. */
    bool is_full() const noexcept
    { return array_state.size() == array_state.count(); }

    /* Work in the cycle. */
    return_list work() noexcept {
        return_list list; /* Return value list.     */
        size_t __cnt = 0; /* Count of ALU occupied. */
        /* This process is actually working parrallelly. */
        for(auto i  = array_state._Find_first() ;
                 i != array_state.size() ; i = array_state._Find_next(i)) {
            if(array[i].is_ready()) { /* Simulate the delay of 1 clock. */
                array_syncs[i]  = false;
                array[i].result = unit[__cnt].work(array[i].src1,
                                                   array[i].src2,
                                                   array[i].op);
                list.push_back({array[i].result,array[i].dest});
            }
        } return list;
    }

    /* Clear the pipeline when prediction fails. */
    void clear_pipeline() noexcept
    { array_state.reset(),array_syncs.set(); }


    /**
     * @brief Receive one command from issue.
     * 
     * @attention Use in the end of the cycle.
     */
    void insert(ALU_code __code,
                wrapper  __reg1,
                wrapper  __reg2,
                register_type __dest) noexcept {
        int __x = (~array_state)._Find_first();
        array_state[__x]= true; /* Set occupied. */
        array[__x].op   = __code;
        array[__x].idx1 = __reg1.index();
        array[__x].idx2 = __reg2.index();
        array[__x].dest = __dest;
        array[__x].src1 = __reg1.value();
        array[__x].src2 = __reg2.value();
    }

    /**
     * @brief Update the dependency from RoB.
     * 
     * @attention Use it in the end of a cycle after inserting.
     */
    void update(wrapper __data) noexcept {
        for(auto i  = array_state._Find_first() ;
                 i != array_state.size() ; i = array_state._Find_next(i)) {
            if(array[i].idx1 == __data.index()) {
                array[i].idx1 = FREE;
                array[i].src1 = __data.value();
            }
            if(array[i].idx2 == __data.index()) {
                array[i].idx2 = FREE;
                array[i].src2 = __data.value();
            }
        }
    }
    

    /**
     * @brief This fucking operation is designed to 
     * simulate real hardware, as real hardware relies
     * on the state of last cycle.
     * 
     * @attention This function should only be operated
     * in the end of the cycle.
     */
    void sync() noexcept { array_state &= array_syncs; array_syncs.set(); }

    /* Return the capacity of the reservation station. */
    constexpr int capacity() const noexcept { return array_length(array); }
};

}

#endif
