#ifndef _RISC_V_PREDICTOR_H_
#define _RISC_V_PREDICTOR_H_

#include "utility.h"


namespace dark {

struct predictor {
    static constexpr uint32_t kAND = 0x0fff;
    static constexpr uint32_t kLEN = 0x1000;
    static constexpr uint32_t next_state[4][2] {
        {1,3},
        {1,0},
        {3,2},
        {0,2},
    };

    struct entry {
        uint32_t data;        /* The state of prediction. */
        uint32_t pattern : 4; /* Real patterns. */
        uint32_t predict : 4; /* Pattern with predictions. */

        /* Set the state for an entry. */
        void set_state(bool result) noexcept {
            uint32_t __len  = pattern << 1;
            uint32_t __cur  = (data >> __len) & 0b11;
            uint32_t __nxt  = next_state[__cur][result];
            data &= ~(0b11u << __len);
            data |=   __nxt << __len ;
            pattern = pattern << 1 | result;
        }

        /* Tries to predict one going.  */
        bool try_predict() noexcept {
            uint32_t __len  = predict << 1 | 1;
            bool result = (data & (1 << __len));
            predict = predict << 1 | result;
            return result;
        }

        /* Clear the prediction. */
        void clear() noexcept { predict = pattern; }
    };

    entry mapping[kLEN];                        /* Mapping of a pc address. */
    round_queue <address_type,32> uncommited;   /* Uncommited pc address.   */
    size_t count[2] = {0,0};                    /* 0 Accurate || 1 Wrong.   */

    /* Predict according to pc. */
    bool predict(address_type __pc) noexcept {
        uncommited.push({__pc &= kAND});
        return mapping[__pc].try_predict();
    }

    /* Update the prediciton from commit message. */
    void update_prediction(bool wrong,bool result)
    noexcept {
        ++count[wrong];
        // return;
        mapping[uncommited.front()].set_state(result);
        if(!wrong) uncommited.pop();
        else { /* The prediction is wrong! */
            int head = uncommited.head;
            int size = uncommited.dist;
            while(size--) {
                mapping[uncommited[head]].clear();
                if(++head == uncommited.length()) head = 0;
            } uncommited.clear();
        }
    }

    /* Get the accuracy for reference. */
    double get_accuracy() const noexcept
    { return static_cast <double> (count[0]) / (count[0] + count[1]); }

    /* Return the total of branches meeting. */
    size_t branches() const noexcept { return count[0] + count[1]; }

    /* Return the capacity of the predictor. */
    constexpr int capacity() const noexcept { return uncommited.length(); }
};


}

#endif
