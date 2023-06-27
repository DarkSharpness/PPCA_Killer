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

    constexpr static byte_utype free = -1;
    byte_utype nxt[32]; /* Dependency of register. */

    /* Intialization. */
    register_file() {
        memset(reg, 0,sizeof(reg));  
        memset(nxt,-1,sizeof(nxt)); /* Free. */
    }

    /**
     * @brief Commit a order from reorder buffer.
     * 
     * @param __pos Index in the reorder buffer.
     * @param __new Value of the new register data.
     */
    void commit(byte_utype __pos,register_type __new) noexcept {
        /* This process is actually working parallelly. */
        for(auto i = 0 ; i != array_length(reg) ; ++i) {
            if(nxt[i] == __pos) {
                nxt[i] = free;
                reg[i] = __new;
            }
        }
    }

    /**
     * @brief Marking one register as occupied.
     * 
     * @param __idx Index of the register.
     * @param __pos Index in the reorder buffer.
     */
    void insert(byte_utype __idx,byte_utype __pos) 
    noexcept { nxt[__idx] = __pos; }

    /* Whether the current register is busy. (-1 -> not busy) */
    wrapper reorder(byte_utype __pos) const noexcept
    { return {reg[__pos],sign_expand <8,uint32_t>(nxt[__pos])}; }
};

}

#endif
