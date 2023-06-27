#ifndef _RISC_V_RESERVATION_H_
#define _RISC_V_RESERVATION_H_

#include "utility.h"
#include "alu.h"

namespace dark {

/* Station for instructions. */
struct reservation_station {
    struct entry {
        byte_utype busy : 1;    /* Busy bit. */
        code_type   op  : 7;    /* Operator bit. */
        byte_utype idx1 : 5;    /* Index of constraint 1 order. */
        byte_utype idx2 : 5;    /* Index of constraint 2 order.  */
        byte_utype dest : 5;    /* Index of destination in reorder buffer. */
        byte_utype      : 0;
        register_type src1;     /* Source value 1. */
        register_type src2;     /* Source value 2.  */
        register_type result;   /* The result of reservation station. */

        /* Whether this entry is available to be executed. */
        bool avail() const noexcept
        { return busy && idx1 != 31 && idx2 != 31; }
    };
    static_assert(sizeof(entry) == 16);

    entry array[32];
    ALU_type ;

    /* A wire indicating whether the arithmetic station is full. */
    bool arith_full() const noexcept {
        for(auto &&[unit,state] : arith)
            if(!state.busy) return false;
        return true;
    }

    /* A wire indicating whether the arithmetic station is full. */
    bool logic_full() const noexcept {
        for(auto &&[unit,state] : logic)
            if(!state.busy) return false;
        return true;
    }

    /* Work in one cycle. */
    return_list work() noexcept {
        return_list list; /* Return value list. */
        /* This process is actually working parrallelly. */
        for(auto &&[unit,state] : arith) {
            if(!state.avail()) continue;
            auto *__p = unit.work(state.src1,state.src2,state.op);
            if(__p) list.push_back({*__p,state.dest}),state.busy = false;
        }
        /* This process is actually working parrallelly. */
        for(auto &&[unit,state] : logic) {
            if(!state.avail()) continue;
            auto *__p = unit.work(state.src1,state.src2,state.op);
            if(__p) list.push_back({*__p,state.dest}),state.busy = false;
        } return list;
    }

    void insert() noexcept {

    }

};

}

#endif
