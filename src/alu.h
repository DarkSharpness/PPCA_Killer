#ifndef _RISC_V_ALU_H_
#define _RISC_V_ALU_H_

#include "utility.h"

namespace dark {

/* Base class for ALU. */
struct arithetic_logic_unit {
    register_type result; /* Result of current operation. */

    register_type work (register_type __reg1,
                        register_type __reg2,
                        alu_code      __code) noexcept {
        switch(__code) {
            case alu_code::add : return             __reg1  + __reg2;
            case alu_code::sub : return             __reg1  - __reg2;
            case alu_code::sll : return (word_utype)__reg1 << __reg2;
            case alu_code::srl : return (word_utype)__reg1 >> __reg2;
            case alu_code::sra : return (word_stype)__reg1 >> __reg2;
            case alu_code::slt : return (word_stype)__reg1  < (word_stype)__reg2;
            case alu_code::sltu: return (word_utype)__reg1  < (word_utype)__reg2;
            case alu_code::xor_: return             __reg1  ^ __reg2;
            case alu_code::or_ : return             __reg1  | __reg2;
            case alu_code::and_: return             __reg1  & __reg2;
            default: ;/* This should never happen. */
        }
    }
};

using ALU_type = arithetic_logic_unit;



}

#endif

