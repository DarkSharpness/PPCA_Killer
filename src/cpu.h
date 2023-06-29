#ifndef _RISC_V_CPU_H_
#define _RISC_V_CPU_H_

#include "bus.h"
#include "memio.h"
#include "reorder.h"
#include "register.h"
#include "instruction.h"
#include "reservation.h"

namespace dark {

/**
 * @brief A simple CPU simulator with instruction unit.
 * 
 */
struct cpu : memory,register_file,reservation_station,reorder_buffer {
    bus            flow;        /* Data flow. */
    size_t        clock;        /* Internal clock. */
    instruction current;        /* Current command. */
    instruction nextcmd;        /* New instruction to fetch. */

    bool  prediction_cur; /* Prediction from this cycle. */
    bool  prediction_pre; /* Prediction from prev cycle. */

    bool   jalr_lock = 0; /* Whether there is a jalr command issued. */

    bool   fetch_pre = 0; /* Whether fetch is available.  */
    bool   issue_pre = 0; /* Whether issue is available. */

    bool   fetch_cur = 0; /* Whether current fetch work.  */
    bool   issue_cur = 0; /* Whether current issue work. */

    address_type  pc_pre   = 0; /* PC in last cycle. */
    address_type  pc_delta = 0; /* The delta of PC in each cycle. */

    /* CPU init. */
    cpu() { memory::init(); }

    /* Whether the command  */
    bool is_terminal() const noexcept
    { return jalr_lock && reorder_buffer::empty(); }

    /* Whether to jump or not. */
    bool predict() const noexcept { return false; }

    /* Whether the command is issuable. */
    bool issueable() const noexcept {
        switch(current.suc) {
            case suc_code::jal   :
            case suc_code::lui   :
            case suc_code::auipc :
            case suc_code::scode :
                return true;
            case suc_code::lcode :
                return memory::is_full();
            default :
                return reservation_station::is_full();
        }
    }

    /**
     * @brief Do fetch operation iff not locked or full.
     * 
     */
    void work_fetch() noexcept {
        if(jalr_lock) return void(fetch_cur = false);
        fetch_cur = true;
        fetch(nextcmd.command);
        if(nextcmd.suc == suc_code::jal) {
            pc_delta = nextcmd.J_immediate();
        } else if(nextcmd.suc == suc_code::bcode) {
            if(bool(prediction_cur = predict()))
                 pc_delta = nextcmd.B_immediate();
            else pc_delta = 4;
        } else   pc_delta = 4;
    }

    /**
     * @brief Reset PC when BRANCH failed or JALR.
     * 
     */
    void reset_pc(address_type __pc) noexcept { pc = __pc; }

    /* Work in one cycle. */
    bool work() noexcept {
        ++clock;
        work_fetch();
        flow.memory_catch(memory::work());
        flow.reorder_catch(reorder_buffer::work());
        flow.reservation_catch(reservation_station::work());
        global_sync();
        return is_terminal();
    }

    /* Clear all the instruction when prediction fail. */
    void clear_instruction() noexcept
    { jalr_lock = fetch_cur = fetch_cur = issue_pre = issue_cur = false; }

    /**
     * @brief Issue one command at a time.
     * 
     * @attention Use it in the end of a cycle.
     */
    void sync_issue() noexcept {
        if(!fetch_pre || !issueable()) return void(issue_cur = false);
        issue_cur = true;
        /* Terminal command case. */
        if(current.command == 0x0ff00513) {
            jalr_lock = true;
            fetch_cur = false;
            issue_cur = false;
            return;
        }
        word_utype __arg  = 0;
        word_utype __tag  = REG_TAG;
        word_utype __dest = current.rd;
        word_utype __done = false;

        switch(current.suc) {
            case suc_code::lcode :
                memory::insert(
                    current.mid,
                    reorder_buffer::buffer_tail(),
                    current.I_imm_11_00,
                    register_file::reorder(current.rs1)
                ); break;

            case suc_code::scode :
                __arg  = current.command;
                __tag  = STORE_TAG;
                __dest = 0;
                __done = true;
                memory::insert_store(reorder_buffer::buffer_tail());
                break;

            case suc_code::bcode :
                __tag  = BRANCH_TAG;
                __arg  = pc_pre;
                __dest = prediction_pre;
                reservation_station::insert(
                    B_ALU_map[current.mid],
                    register_file::reorder(current.rs1),
                    register_file::reorder(current.rs2),
                    reorder_buffer::buffer_tail()
                ); break;

            case suc_code::icode :
                ALU_code __code = (ALU_code)current.mid;
                if(__code == ALU_code::SRL && current.pre)
                    __code = ALU_code::SRA , current.pre = 0;
                reservation_station::insert(
                    __code,
                    register_file::reorder(current.rs1),
                    {current.I_imm_11_00,FREE},
                    reorder_buffer::buffer_tail()
                ); break;

            case suc_code::rcode :
                ALU_code __code = (ALU_code)current.mid;
                if(__code == ALU_code::SRL && current.pre)
                    __code = ALU_code::SRA;
                if(__code == ALU_code::ADD && current.pre)
                    __code = ALU_code::SUB;
                reservation_station::insert(
                    __code,
                    register_file::reorder(current.rs1),
                    register_file::reorder(current.rs2),
                    reorder_buffer::buffer_tail()
                ); break;

            case suc_code::jalr  :
                jalr_lock = true;
                __tag  = JALR_TAG;
                break;

            case suc_code::jal   :
                __arg  = pc_pre + current.J_immediate();
                __done = true;
                break;
    
            case suc_code::auipc : __arg = pc_pre;
            case suc_code::lui   :
                __arg += current.U_imm_31_12;
                __done = true;
                break; /* Original command. */

            default: throw; /* This should never happen. */
        } reorder_buffer::insert(__arg,__tag,__dest,__done);
    }

    /* Flush the bus data. */
    void flush_bus() noexcept {
        /* Commit message is not empty. */
        if(!flow.ReG_update.is_empty()) {
            

        }

        /* RoB need updating. */
        for(auto &&iter : flow.RoB_update) {
            
        }
    }

    inline void sync_bus() noexcept {
        flush_bus();
        flow.clear();
    }

    void sync_instruction() noexcept {
        /* Only when issue success and fetch sucess. */
        if(issue_cur && fetch_cur) {
            current        = nextcmd;
            prediction_pre = prediction_cur;
            pc_pre         = pc;
            pc            += pc_delta;
        } /* Update both tags. */
        issue_pre = issue_cur;
        fetch_pre = fetch_cur;
    }

    /* Global synchronize. */
    void global_sync() noexcept {
        sync_issue();
        sync_bus();
        sync_instruction();
        /* Order of the 3 instructions above can't change! */
        memory::sync();
        reorder_buffer::sync();
        reservation_station::sync();
    }

};

}

#endif
