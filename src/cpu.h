#ifndef _RISC_V_CPU_H_
#define _RISC_V_CPU_H_

#include "bus.h"
#include "memio.h"
#include "reorder.h"
#include "register.h"
#include "instruction.h"
#include "reservation.h"
#include "predictor.h"

#include <functional>
#include <random>

namespace dark {

/**
 * @brief A simple CPU simulator with instruction unit.
 * 
 */
struct cpu : memory,register_file,reservation_station,reorder_buffer,predictor {
    bus            flow;        /* Data flow. */
    size_t        clock;        /* Internal clock. */
    instruction current;        /* Current command. */
    instruction nextcmd;        /* New instruction to fetch. */

    bool  prediction_cur; /* Prediction from this cycle. */
    bool  prediction_pre; /* Prediction from prev cycle. */

    bool   jalr_lock = 0; /* Whether there is a jalr command issued. */
    bool   full_lock = 0; /* Whether this issue is blocked by full. */

    bool   fetch_pre = 0; /* Whether fetch is available.  */
    bool   fetch_cur = 0; /* Whether current fetch work.  */

    address_type  pc_pre   = 0; /* PC in last cycle. */
    address_type  pc_delta = 0; /* The delta of PC in each cycle. */

    size_t prediction_count; /* Count of all predictions. */
    size_t prediction_wrong; /* Wrong rate. */


    /* Whether the command  */
    bool is_terminal() const noexcept
    { return jalr_lock && reorder_buffer::empty(); }

    /* Whether the command is issuable. */
    bool issueable() const noexcept { return !reorder_buffer::is_full(); }

    /**
     * @brief Do fetch operation iff not locked.
     * 
     */
    void work_fetch() noexcept {
        if(jalr_lock) return void(fetch_cur = false);
        fetch_cur = true;       /* This tag may go invalid in future. */
        if(full_lock) return;   /* Locked by full,so no need fetching. */
        fetch(nextcmd.command); /* Fetch one command at a time. */

        if(nextcmd.suc == suc_code::jal) {
            pc_delta = nextcmd.J_immediate();
        } else if(nextcmd.suc == suc_code::bcode) {
            if(bool(prediction_cur = predict(pc)))
                pc_delta = nextcmd.B_immediate();
            else /* No jump case. */
                pc_delta = 4;
        } else pc_delta = 4;
    }

    /**
     * @brief Reset PC when BRANCH failed or JALR.
     * 
     */
    void reset_pc(address_type __pc) noexcept { pc = __pc; }

    /* Clear all the instruction when prediction fail. */
    void clear_instruction() noexcept
    { jalr_lock = fetch_pre = fetch_cur = full_lock = false; }

    /**
     * @brief Synchronize the one command issued at a time.
     * 
     * @attention Use it in the end of a cycle.
     */
    void sync_issue() noexcept {
        if(!fetch_pre)   return void(full_lock = false);
        if(!issueable()) return void(full_lock = true );

        /* Terminal command case. */
        if(current.command == 0x0ff00513) {
            jalr_lock = true;
            full_lock = true;
            fetch_cur = false;
            return;
        }   full_lock = false;

        word_utype __arg  = 0;
        word_utype __tag  = REG_TAG;
        word_utype __dest = current.rd;
        word_utype __done = false;
        word_utype __tail = reorder_buffer::buffer_tail();
        ALU_code   __code = (ALU_code)current.mid;

        switch(current.suc) {
            case suc_code::lcode :
                memory::insert(
                    current.mid,
                    __tail,
                    current.I_immediate(),
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
                __arg  = pc_pre + (prediction_pre ? 4 : current.B_immediate());
                __dest = prediction_pre;
                reservation_station::insert(
                    B_ALU_map[current.mid],
                    register_file::reorder(current.rs1),
                    register_file::reorder(current.rs2),
                    reorder_buffer::buffer_tail()
                ); break;

            case suc_code::rcode :
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

            case suc_code::jalr  :  /* Special immediate command. */
                jalr_lock = true;   /* Trigger lock. */
                fetch_cur = false;  /* Current fecth becomes invalid. */
                __tag     = JALR_TAG;
                __code    = ALU_code::ADD;
            case suc_code::icode :
                if(__code == ALU_code::SRL && current.pre)
                    __code = ALU_code::SRA , current.pre = 0;
                reservation_station::insert(
                    __code,
                    register_file::reorder(current.rs1),
                    wrapper{current.I_immediate(),FREE},
                    reorder_buffer::buffer_tail()
                ); break;

            case suc_code::jal   :
                __arg  = pc_pre + 4;
                __done = true;
                break;

            case suc_code::auipc : __arg = pc_pre;
            case suc_code::lui   :
                __arg += current.U_immediate();
                __done = true;
                break; /* Original command. */

            /* Mistaken prediction with wrong address. */
            default: return void(full_lock = true); 
        }

        /* Require updating register. */
        if(__tag == REG_TAG || __tag == JALR_TAG)
            register_file::insert(
                __dest,
                reorder_buffer::buffer_tail()
            );
        reorder_buffer::insert(__arg,__tag,__dest,__done);
    }

    /* Flush the bus data. */
    void sync_bus() noexcept {
        /* Commit message is not empty. */
        int __head = reorder_buffer::buffer_head();
        if(!flow.ReG_update.is_empty()) {
            switch(flow.ReG_update.tag()) {
                case JALR_TAG   :
                    jalr_lock = false;
                    reset_pc(flow.ReG_update.pc());
                    flow.ReG_update.val = pc_pre + 4;
                case REG_TAG    :
                    register_file::commit(
                        flow.ReG_update.index(),
                        __head,
                        flow.ReG_update.value()
                    ); flow.ReG_update.idx = __head;
                    reservation_station::update(flow.ReG_update);
                    memory::update(flow.ReG_update); break;

                case BRANCH_TAG :
                    predictor::update_prediction(
                        flow.ReG_update.is_wrong(),
                        flow.ReG_update.result()
                    );
                    if(flow.ReG_update.is_wrong()) {
                        reset_pc(flow.ReG_update.pc());
                        return global_clear();
                    } break;

                case STORE_TAG  : {
                    instruction __inst = {flow.ReG_update.value()};
                    memory::store(
                        __inst.mid,
                        __head,
                        register_file::reg[__inst.rs1] + __inst.S_immediate(),
                        register_file::reg[__inst.rs2]
                    );
                } break;

                default: ;/* This should never happen. */
            }
        } /* Reorder buffer may need updating. */
        reorder_buffer::update(flow.RoB_update);
        flow.clear();
    }

    /* Synchronize the insturction unit. */
    void sync_instruction() noexcept {
        /* Only when issue success and fetch sucess. */
        if(fetch_cur && !full_lock) {
            current        = nextcmd;
            prediction_pre = prediction_cur;
            pc_pre         = pc;
            pc            += pc_delta;
        } /* Update both tags. */
        fetch_pre = fetch_cur;
    }

    /* Clear all the pipelines. */
    void global_clear() noexcept {
        flow.clear();
        clear_instruction();
        memory::clear_pipeline();
        register_file::clear_pipeline();
        reorder_buffer::clear_pipeline();
        reservation_station::clear_pipeline();
    }

    /* Global synchronize. */
    void global_sync() noexcept {
        sync_issue();
        sync_bus();
        sync_instruction();
        /* Order of 3 functions above can't change! */
        memory::sync();
        reorder_buffer::sync();
        reservation_station::sync();
    }

    /* Work in one cycle. */
    bool work() noexcept {
        static std::function <void()> func[] = {
            [&]() {work_fetch();},
            [&]() {flow.memory_catch(memory::work());},
            [&]() {flow.reorder_catch(reorder_buffer::work());},
            [&]() {flow.reservation_catch(reservation_station::work());},
        };
        static int order[4] = {0,1,2,3};
        // static std::random_device abelcat;
        ++clock;


        // std::shuffle(order,order + array_length(order),abelcat);
        for(size_t i = 0 ; i != array_length(order) ; ++i)
            std::swap(order[i],order[i % array_length(order)]);

        for(size_t i = 0 ; i != array_length(order) ; ++i)
            func[order[i]]();

        // work_fetch();
        // flow.memory_catch(memory::work());
        // flow.reorder_catch(reorder_buffer::work());
        // flow.reservation_catch(reservation_station::work());

        /* Synchronize to simulate hardware. */   
        global_sync();
        return !is_terminal();
    }

};

}

#endif
