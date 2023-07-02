#ifndef _RISC_V_INSTURCTION_H_
#define _RISC_V_INSTURCTION_H_

#include "utility.h"

namespace dark {

/**
 * @brief The instruction union class.
 * 
 */
struct instruction {
    union {
        command_type command; /* The whole command. */
        struct { /* Common data. */
            command_type     : 7;   /* Padding. */
            command_type rd  : 5;   /* Register destination part. */
            command_type mid : 3;   /* Mid code part. */
            command_type rs1 : 5;   /* Register 1.    */
            command_type rs2 : 5;   /* Register 2.    */
            command_type pre : 7;   /* Pre code part. */
        };
        struct { /* Common data. */
            suc_code suc : 7; /* Suc code part. */ 
        };

        struct { /* U command. */
            command_type             : 12;
            command_type U_imm_31_12 : 20; 
        };
        struct { /* J command. */
            command_type             : 12;
            command_type J_imm_19_12 :  8;
            command_type J_imm_11_11 :  1;
            command_type J_imm_10_01 : 10;
            command_type J_imm_20_20 :  1;
        };
        struct { /* I command. */
            command_type             : 20;
            command_type I_imm_11_00 : 12;
        };
        struct { /* S command. */
            command_type             :  7;
            command_type S_imm_04_00 :  5;
            command_type             : 13;
            command_type S_imm_11_05 :  7;
        };
        struct { /* B command. */
            command_type             :  7;
            command_type B_imm_11_11 :  1;
            command_type B_imm_04_01 :  4;
            command_type             : 13;
            command_type B_imm_10_05 :  6;
            command_type B_imm_12_12 :  1;
        };
    };

    /* Full immediate number. */
    word_stype B_immediate() const noexcept {
        return
            sign_expand <13,address_type> (
                B_imm_12_12 << 12 |
                B_imm_11_11 << 11 |
                B_imm_10_05 <<  5 |
                B_imm_04_01 <<  1
            );
    }

    /* Full immediate number. */
    address_type J_immediate() const noexcept {
        return
            sign_expand <21,address_type> (
                J_imm_20_20 << 20 |
                J_imm_19_12 << 12 |
                J_imm_11_11 << 11 |
                J_imm_10_01 <<  1
            );
    }

    /* Full immediate number. */
    address_type U_immediate() const noexcept
    { return U_imm_31_12 << 12; }

    address_type I_immediate() const noexcept
    { return sign_expand <12,address_type> (I_imm_11_00); }

    /* Full immediate number. */
    address_type S_immediate() const noexcept
    { return sign_expand <12,address_type> (S_imm_11_05 << 5 | S_imm_04_00 << 0); }
}; static_assert(sizeof(instruction) == sizeof(command_type));

}

#endif
