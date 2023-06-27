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
            command_type suc : 7;   /* Suc code part. */
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
};

static_assert(sizeof(instruction) == sizeof(command_type));

struct instruction_fetcher {

    void fetch_instruction() {


    }
};


}

#endif
