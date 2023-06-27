#ifndef _RISC_V_MEMIO_H_
#define _RISC_V_MEMIO_H_

#include "utility.h"
#include "memchip.h"

namespace dark {


constexpr address_type memory_size = 1 << 20;

/**
 * @brief A buffered fixed-sized memory_chip.
 * 
 */
struct memory : memory_chip <memory_size> {
    /* Entry of one memory buffer. */
    struct entry {
        word_utype busy   :  1; /* Busy bit || Load bit. */
        word_utype dest   :  5; /* Index in the reorder buffer. */
        word_utype count  :  2; /* Counter of the clock. */
        word_utype size   :  3; /* The length of data.  */
        word_utype index  :  5; /* Index of constraint1.  */
        word_utype offset : 13; /* Offset of address (with additional sign bit) */
        word_utype        :  0;
        register_type   source; /* The source register value. */
        register_type   result; /* Result of the calculation. */

        /* Whether  */
        bool is_load() const noexcept { return busy; }

        /* Whether this entry is available to be executed. */
        bool avail() const noexcept { return busy && index != 31; }

        /* Return the real address. */
        address_type address() const noexcept
        { return source + sign_expand <13,address_type> (offset); }

    };
    static_assert(sizeof(entry) == 12);

    round_queue <entry,32> loader; /* Load  buffer. */

    entry current = {0,0,0,0,0,0,0,0};
    entry tostore = {0,0,0,0,0,0,0,0};

    bool sync_tag_loader = false;
    bool sync_tag_storer = false;

    /**
     * @brief Inner method of fetching a command.
     * Note that this command is only used in C++
     * simulation, as decode center should directly
     * fetch command from memory chip.
     * 
     * @param __pc The real PC value.
     * @return command_type 
     */
    command_type fetch(address_type __pc) noexcept {
        command_type __ret;
        memory_chip::load(__pc,__ret);
        return __ret;
    }

    /**
     * @brief Load one data (into queue first).
     * 
     * @param __add Address in the memory.
     * @param __pos Index in reorder buffer. 
     */
    template <class T>
    void load(address_type __add,byte_utype __pos,T) 
    noexcept { loader.push({__add,0,sizeof(T),1,3,__pos}); }

    /**
     * @brief Store one data.
     * 
     * @param __add Address in the memory.
     * @param __pos Index in reorder buffer. 
     * @param __arg Argument to store into memory.
     */
    template <class T>
    void store(address_type __add,byte_utype __pos,T __arg) 
    noexcept { storer.push({__add,__arg,sizeof(T),0,3,__pos}); }

    /* Load one data from memory. */
    bool load_current() noexcept {
        memory_chip::load (current.address(),current.result,current.size);
        return true;
    }

    /* Store one data into memory. */
    bool store_current() noexcept {
        memory_chip::store(current.address(),current.result,current.size);
        return false;
    }

    /* Work for one cycle. Return whether information should be updated. */
    wrapper work() noexcept {
        if(!current.count) { /* Wired operation. */
            
        } if(current.count != 1) return {};
        /* Start operating. */
    
    }

    /* A wire indicating whether the loader is full. */
    bool loader_full() const noexcept { return loader.full(); }

    /* A write indicating whether the memory is writeable. */
    bool writeable() const noexcept {
        return current.count == 1 || /* Will be writeable or empty. */
              (current.count == 0 && loader.empty());
    }



    /**
     * @brief This fucking operation is designed to 
     * simulate real hardware, as real hardware relies
     * on the state of last cycle.
     * 
     */
    void sync() {
        if(current.count) --current.count;
        if(sync_tag_loader) loader.pop() , sync_tag_loader = false;
    }

};

}

#endif
