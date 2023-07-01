#ifndef _RISC_V_REGISTER_H_
#define _RISC_V_REGISTER_H_

#include "utility.h"

namespace dark {

/* Register status holder. */
struct register_file {
    union {
        register_type reg[32];
        struct { /* Specific registers. */
            register_type zero; /* Hardwired 0. */
            register_type ra;   /* Return address.  */
            register_type sp;   /* Stack pointer. */
            register_type gp;   /* Global pointer. */
            register_type tp;   /* Thread pointer. */
            register_type t0;   /* Temporary. */
            register_type t1;   /* Temporary. */
            register_type t2;   /* Temporary. */
            union {
                register_type s0;   /* Saved register. */
                register_type fp;   /* Frame pointer. */  
            };
            register_type s1;   /* Saved register. */
            register_type a0;   /* Function argument,return value. */
            register_type a1;   /* Function argument,return value. */
            register_type a2;   /* Function arugment. */        
            register_type a3;   /* Function arugment. */        
            register_type a4;   /* Function arugment. */        
            register_type a5;   /* Function arugment. */        
            register_type a6;   /* Function arugment. */        
            register_type a7;   /* Function arugment. */        
            register_type s2;   /* Saved register. */
            register_type s3;   /* Saved register. */
            register_type s4;   /* Saved register. */
            register_type s5;   /* Saved register. */
            register_type s6;   /* Saved register. */
            register_type s7;   /* Saved register. */
            register_type s8;   /* Saved register. */
            register_type s9;   /* Saved register. */
            register_type s10;  /* Saved register. */
            register_type s11;  /* Saved register. */
            register_type t3;   /* Temoporary. */
            register_type t4;   /* Temoporary. */
            register_type t5;   /* Temoporary. */
            register_type t6;   /* Temoporary. */
        };
    };

    byte_utype nxt[32]; /* Dependency of register. */

    /* Intialization. */
    register_file() noexcept {
        memset(reg,  0 ,sizeof(reg));  
        memset(nxt,FREE,sizeof(nxt)); /* Free. */
    }

    /**
     * @brief Commit a order from reorder buffer.
     * 
     * @param __idx Index in the register file.
     * @param __pos Index in the reorder buffer.
     * @param __new Value of the new register data.
     */
    void commit(word_utype __idx,word_utype __pos,
                register_type __new) noexcept {
        if(__idx) reg[__idx] = __new;
        if(nxt[__idx] == __pos) nxt[__idx] = FREE;
    }

    /**
     * @brief Marking one register as occupied.
     * Note that 0 register won't be occupied.
     * 
     * @param __idx Index of the register.
     * @param __pos Index in the reorder buffer.
     */
    void insert(word_utype __idx,byte_utype __pos)
    noexcept { if(__idx) nxt[__idx] = __pos; }

    /* Whether the current register is busy. (-1 -> not busy) */
    wrapper reorder(byte_utype __pos) const noexcept
    { return {reg[__pos],nxt[__pos]}; }

    /* Clear the pipeline when prediction fails. */
    void clear_pipeline() noexcept { memset(nxt,FREE,sizeof(nxt)); }
};

}

#endif
