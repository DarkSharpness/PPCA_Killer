#ifndef _RISC_V_BUS_H_
#define _RISC_V_BUS_H_

#include "utility.h"

namespace dark {


/**
 * @brief A bus class holding all none immediate signal.
 * 
 */
struct bus {
    return_list RoB_update;         /* Update reorder buffer. */
    wrapper     ReG_update = {0,0}; /* Update register file and RS and LSB. */

    /**
     * @brief Catch the signal from reservation station.
     * It will update RoB in the end of a cycle.
     * 
     * @param __list List of updates.
     */
    void reservation_catch(const return_list &__list) noexcept
    { for(auto &&iter : __list) RoB_update.push_back(iter); }

    /**
     * @brief Catch the signal from load store buffer.
     * It will update RoB in the end of a cycle.
     * 
     * @param __list List of updates.
     */
    void memory_catch(const return_list &__data)
    noexcept { reservation_catch(__data); }

    /**
     * @brief Catch the signal from reorder buffer.
     * It will update LSB,RF,RS in the end of a cycle.
     * 
     * @param __list List of updates.
     */
    void reorder_catch(wrapper __data) noexcept { ReG_update = __data; }

    /**
     * @brief Clear everything 
     * 
     * @attention Work in the end of a cycle, after inner data is used.
     */
    void clear() noexcept { RoB_update.clear(); ReG_update = {0,0}; }
};



}


#endif
