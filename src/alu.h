#ifndef _RISC_V_ALU_H_
#define _RISC_V_ALU_H_

#include "utility.h"

namespace dark {

/**
 * @brief Custom class for ALU operation.
 * Note that it is implemented as an empty class.
 * You should use [[no_unique_address]] to save space.
 * 
 */
struct arithetic_logic_unit {
    /* It is actually empty class which requires no this pointer. */
    static register_type work (register_type __reg1,
                               register_type __reg2,
                               ALU_code      __code) noexcept {
        switch(__code) {
            case ALU_code::ADD : return             __reg1  + __reg2;
            case ALU_code::SUB : return             __reg1  - __reg2;
            case ALU_code::ALL : return (word_utype)__reg1 << __reg2;
            case ALU_code::SRL : return (word_utype)__reg1 >> __reg2;
            case ALU_code::SRA : return (word_stype)__reg1 >> __reg2;
            case ALU_code::LT  : return (word_stype)__reg1  < (word_stype)__reg2;
            case ALU_code::LTU : return (word_utype)__reg1  < (word_utype)__reg2;
            case ALU_code::GE  : return (word_stype)__reg1 >= (word_stype)__reg2;
            case ALU_code::GEU : return (word_utype)__reg1 >= (word_utype)__reg2;
            case ALU_code::EQ  : return             __reg1 == __reg2;
            case ALU_code::NE  : return             __reg1 != __reg2;
            case ALU_code::XOR: return             __reg1  ^ __reg2;
            case ALU_code::OR : return             __reg1  | __reg2;
            case ALU_code::AND: return             __reg1  & __reg2;
            default: throw;/* This should never happen. */
        }
    }
};

using ALU_type = arithetic_logic_unit;



}

#endif

