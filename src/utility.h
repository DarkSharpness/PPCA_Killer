#ifndef _RISC_V_UTILITY_H_
#define _RISC_V_UTILITY_H_

#include "round_queue.h"

#include <stdint.h>
#include <string.h>
#include <locale>
#include <vector>
#include <bitset>
#include <iostream>


namespace dark {


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

/* Mid : bit 14 ~ 12*/
enum class mid_code : int8_t {
    B_type, /* ALU code. */
    I_type, /* ALU code. */
    R_type, /* ALU code. */
    L_type, /* Memory code. */
    S_type, /* Memory code. */
};

/* Mid : bit 14 ~ 12  */
enum class ALU_code : int8_t {
    ADD =  0b000,
    SUB = ~0b000,
    ALL =  0b001,
    XOR =  0b100,
    SRL =  0b101,
    SRA = ~0b101,
    OR  =  0b110,
    AND =  0b111,
    EQ  = ~0b111,
    NE  = ~0b110,
    LT  =  0b010,
    GE  = ~0b010,
    LTU =  0b011,
    GEU = ~0b011,

    WORKING = ~0b100  /* Magic number. */
};

/* Branch code should be remapped. */
constexpr ALU_code B_ALU_map[] = {
    ALU_code::EQ,
    ALU_code::NE,
    ALU_code::WORKING, /* Empty. */
    ALU_code::WORKING, /* Empty. */
    ALU_code::LT,
    ALU_code::GE,
    ALU_code::LTU,
    ALU_code::GEU,
};

/* Mid : bit 14 ~ 12  */
enum class MEM_code : uint8_t {
    byte  = 0b000,
    half  = 0b001,
    word  = 0b010,
    ubyte = 0b100,
    uhalf = 0b101,
};

constexpr uint32_t       FREE = 31; /* Maximum available in RoB. */
constexpr uint32_t    REG_TAG = 0;  /* Noraml command. */ 
constexpr uint32_t   JALR_TAG = 1;  /* JALR.   */
constexpr uint32_t  STORE_TAG = 2;  /* JALR.   */
constexpr uint32_t BRANCH_TAG = 3;  /* B-type. */

/* Simple wrapper of bus value. */
struct wrapper { /* 32 + 7 bits */
    uint32_t val; /* Value of the register. */
    uint32_t idx; /* Index of the register/RoB. */

    /* Return the index in reorder buffer. */
    uint32_t index() const noexcept { return idx & FREE; }
    /* Return the value from given reorder.(is_file() == true) */
    uint32_t value() const noexcept { return val; }
 
    /* Return pc pointer from given reorder.(is_file() == true) */
    uint32_t pc()    const noexcept { return val & ~1; }
    /* Return the tagging. */
    uint32_t tag()   const noexcept { return idx >> 5; }

    /* Whether to store into register file. */
    bool is_file()    const noexcept { return tag() == 0; }
    /* Whether a  jalr command. */
    bool is_jalr()    const noexcept { return tag() == 1; }
    /* Whether a store command. */
    bool is_store()   const noexcept { return tag() == 2; }
    /* Previous prediction (True / False). */
    bool prediction() const noexcept { return idx & 1; }
    /* Whether prediction is true. */
    bool is_wrong()   const noexcept { return (val ^ idx) & 1; }
    /* Whether jump successfully. */
    bool is_jump_AC() const noexcept { return tag() == 3 && is_wrong() == 0; }
    /* Whether jump wrongly. */
    bool is_jump_WA() const noexcept { return tag() == 3 && is_wrong() == 1; }
    /* Whether the commit message is empty. */
    bool is_empty()   const noexcept { return idx == 0; }
};


}


namespace dark {

using word_stype = int32_t;
using half_stype = int16_t;
using byte_stype = int8_t;

using word_utype = uint32_t;
using half_utype = uint16_t;
using byte_utype = uint8_t;

using address_type  = uint32_t;
using command_type  = uint32_t;
using register_type = uint32_t;

using return_list = std::vector <wrapper>;

/* Judge whether given char is a visible char */
bool is_visible_char(int __c) noexcept
{ return __c < 127 && __c > 32; }

/* Read a token. Return whether EOF isn't reached. */
bool read_token(char *__str) noexcept {
    int __c;
    while(!is_visible_char(__c = getchar()))
        if(__c == EOF) return false;
    while(is_visible_char(__c)) {
        *(__str++) = __c;
        __c  = getchar();
    }*__str = 0; return true;
}

/* Map a char into a hex number. */
int char_map(char x) noexcept { return isdigit(x) ? x - '0' : x - 'A' + 10; }

/* Turn a hex number string into a trivial number type. */
template <class T>
T hex_to_integer(char *__str) noexcept {
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
             ((-1u) << __n) | __v : /* Fill upper bits with 1. */
            ~((-1u) << __n) & __v ; /* Fill upper bits with 0. */
}

/* Expand n bit integer by its sign. */
template <size_t __n,class T>
T sign_expand(T __v,bool __bit) noexcept {
    static_assert(__n > 0 && __n < bit_length <T>);
    return __bit == true ? /* Test sign bit of current integer. */
             ((-1u) << __n) | __v : /* Fill upper bits with 1. */
            ~((-1u) << __n) & __v ; /* Fill upper bits with 0. */
}

template <class T,size_t __n>
constexpr size_t array_length(T (&__a)[__n]) { return __n; }


}


#endif
