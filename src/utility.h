#ifndef _RISC_V_UTILITY_H_
#define _RISC_V_UTILITY_H_

#include "round_queue.h"

#include <stdint.h>
#include <string.h>
#include <locale>

namespace dark {

/* The real code marker. */
enum class code : uint8_t {
    lui,    /* Load upper immediate. */
    auipc,  /* Add upper immediate to pc. */
    jal,    /* Jump and link. */
    jalr,   /* Jump and link register. */
    beq,    /* Branch when equal. */
    bne,    /* Branch when not equal. */
    blt,    /* Branch when less than. */
    bge,    /* Branch when greater or equal. */
    bltu,   /* Branch when less than (unsigned). */
    bgeu,   /* Branch when greater or equal (unsigned). */
    lb,     /* Load byte. */
    lh,     /* Load half word. */
    lw,     /* Load word. */
    lbu,    /* Load unsigned byte. */
    lhu,    /* Load unsigned half word. */
    sb,     /* Store byte. */
    sh,     /* Store half word. */
    sw,     /* Store word. */
    addi,   /* Add immediate. */
    slti,   /* Set if less than immediate. */ 
    sltiu,  /* Set if less than immediate(unsigned). */
    xori,   /* Xor immediate. */
    ori,    /* Or  immediate. */
    andi,   /* And immediate. */
    slli,   /* Shift left  logical immediate. */
    srli,   /* Shift right logical immediate. */
    srai,   /* Shift right arithmetic immediate. */
    add,    /* Add 2 registers. */
    sub,    /* Subtract 2 registers. */
    sll,    /* Shift left  logical. */
    slt,    /* Set if less than. */
    sltu,   /* Set if less than unsigned.*/
    xor_,   /* Xor 2 registers. */
    srl,    /* Shift right logical. */
    sra,    /* Shift right arithmetic. */
    or_,    /* Or  2 registers. */
    and_    /* And 2 registers. */
};


/* Suc : Last 7 bit code. */
enum class suc_code : uint8_t {
    lui   = 0b0110111, // Upper
    auipc = 0b0010111, // Upper
    jal   = 0b1101111, // Jump
    jalr  = 0b1100111, // Jump
    bcode = 0b1100011, // Branch
    lcode = 0b0000011, // Load
    scode = 0b0100011, // Store
    icode = 0b0010011, // Immediate
    rcode = 0b0110011  // Arithmetic
};


/* Mid : bit 14 ~ 12  */
enum class alu_code : uint8_t {
    add  =           0b000,
    sub  = (uint8_t)~0b000,
    sll  =           0b001,
    slt  =           0b010,
    sltu =           0b011,
    xor_ =           0b100,
    srl  =           0b101,
    sra  = (uint8_t)~0b101,
    or_  =           0b110,
    and_ =           0b111,
};


/* Mid : bit 14 ~ 12  */
enum class memory_code : uint8_t {
    byte  = 0b000,
    half  = 0b001,
    word  = 0b010,
    ubyte = 0b100,
    uhalf = 0b101,
};


/* Mid : bit 14 ~ 12  */
enum class branch_code : uint8_t {
    beq  = 0b000,
    bne  = 0b001,
    blt  = 0b100,
    bge  = 0b101,
    bltu = 0b110,
    bgeu = 0b111,
};

}


namespace dark {

using code_type = code;

using word_stype = int32_t;
using half_stype = int16_t;
using byte_stype = int8_t;

using word_utype = uint32_t;
using half_utype = uint16_t;
using byte_utype = uint8_t;

using address_type  = uint32_t;
using command_type  = uint32_t;
using register_type = uint32_t;


/* Judge whether given char is a visible char */
bool is_visible_char(int __c) noexcept
{ return __c > 126 || __c < 33; }


/* Read a token. Return whether EOF isn't reached. */
bool read_token(char *__str) noexcept {
    int __c;
    while(is_visible_char(__c = getchar())) {
        *(__str++) = __c;
    } *__str = 0; 
    return __c != EOF;
}


/* Map a char into a hex number. */
int char_map(char x) noexcept { return isdigit(x) ? x - '0' : x - 'A' + 10; }


/* Turn a hex number string into a trivial number type. */
template <class T>
T to_integer(char *__str) noexcept {
    T __v = 0;
    while(*__str) __v = __v << 4 | char_map(*__str++);
    return __v;
}


/* The bit length of an integer. */
template <class T>
constexpr size_t bit_length = sizeof(T) << 3;


/* Expand n bit integer by its sign. */
template <size_t __n,class T>
T sign_expand(T __v) noexcept {
    static_assert(__n > 0 && __n < bit_length <T>);
    return __v & (1 << (__n - 1)) ? /* Test sign bit of current integer. */
             (T(-1) << __n) | __v : /* Fill upper bits with 1. */
            ~(T(-1) << __n) & __v ; /* Fill upper bits with 0. */
}

template <class T,size_t __n>
constexpr size_t array_length(T (&__a)[__n]) { return __n; }

}


#endif
