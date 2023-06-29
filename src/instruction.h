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
            command_type pre : 7;   /* Pre code part. */
            command_type rs2 : 5;   /* Register 2.    */
            command_type rs1 : 5;   /* Register 1.    */
            command_type mid : 3;   /* Mid code part. */
            command_type rd  : 5;   /* Register destination part. */
        };
        struct {
            suc_code ____[3];
            suc_code     : 1;
            suc_code suc : 7; /* Suc code part. */ 
        };

        struct { /* U command. */
            command_type U_imm_31_12 : 20; 
        };
        struct { /* J command. */
            command_type J_imm_20_20 :  1;
            command_type J_imm_10_01 : 10;
            command_type J_imm_11_11 :  1;
            command_type J_imm_19_12 :  8;
        };
        struct { /* I command. */
            command_type I_imm_11_00 : 12;
        };
        struct { /* S command. */
            command_type S_imm_11_05 :  7;
            command_type             : 13;
            command_type S_imm_04_00 :  5;
        };
        struct { /* B command. */
            command_type B_imm_12_12 :  1;
            command_type B_imm_10_05 :  6;
            command_type             : 13;
            command_type B_imm_04_01 :  5;
            command_type B_imm_11_11 :  1;
        };
    };

    /* Full immediate number. */
    address_type B_immediate() const noexcept {
        return
            sign_expand <13,address_type> (
                B_imm_12_12 << 12 |
                B_imm_11_11 << 11 |
                B_imm_10_05 <<  5 |
                B_imm_04_01 <<  1 , B_imm_12_12
            );
    }

    /* Full immediate number. */
    address_type J_immediate() const noexcept {
        return
            sign_expand <21,address_type> (
                J_imm_20_20 << 20 |
                J_imm_19_12 << 12 |
                J_imm_11_11 << 11 |
                J_imm_10_01 <<  1 , J_imm_20_20
            );
    }

    /* 12 bits. */
    address_type S_immediate() const noexcept { return S_imm_11_05 << 5 | S_imm_04_00 << 0 ; }
};

static_assert(sizeof(instruction) == sizeof(command_type));

struct data_pack {
    union {
        command_type command;
        struct { /* Common data. */
            register_type     : 12;
            register_type rs1 :  5; /* Register source 1. */
            register_type     :  3; /* Mid code part. */
            register_type  rd :  5; /* Register destination part. */
        };

        struct { /* B type data. */
            register_type       :  7;
            register_type R_rs2 :  7;
            register_type       :  5; 
            register_type       :  3;
            register_type B_rs2 :  5; /* Register source 2. */
        };

        struct { /* S type data. */
            register_type       :  7;
            register_type       :  7;
            register_type       :  5; 
            register_type       :  3;
            register_type S_rs2 :  5; /* Register source 2. */
        };

        struct { /* Common suc code. */
            ALU_code _____[2]; /* Padding. */
            mid_code      : 1;
            mid_code type : 3;
            mid_code      : 4;
            union {
                struct {
                    ALU_code         : 1;
                    ALU_code ALU_suc : 7;
                };
                struct {
                    MEM_code         : 1;
                    MEM_code MEM_suc : 7;
                };
                struct {
                    suc_code         : 1;
                    suc_code U_J_suc : 7;
                };
            };
        };

        struct { register_type imm_12 : 12; };

        struct { register_type imm_20 : 20; };

    };
};
static_assert(sizeof(data_pack) == sizeof(instruction));


}

#endif
