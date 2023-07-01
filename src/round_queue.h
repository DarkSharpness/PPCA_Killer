#ifndef _RISC_V_QUEUE_H_
#define _RISC_V_QUEUE_H_

namespace dark {


template <class T,int __n>
struct round_queue {
    T data[__n];
    int head = 0;
    int dist = 0;

    /* Clear all the elements in the queue. */
    void clear() /*noexcept*/ { head = dist = 0; }

    /* Force to push an element to the back of the queue. */
    void push(const T &__v) /*noexcept*/ { data[tail()] = __v; ++dist; }

    /* Pop an element from the front of the queue. */
    void pop() /*noexcept*/ { --dist; if(++head == __n) head = 0; }

    /* Return the tail position ready to be inserted. */
    int tail() const /*noexcept*/ {
        int __x = head + dist;
        return __x >= __n ? __x - __n : __x;
    }

    T &operator [](int x) /*noexcept*/ { return data[x]; }

    /* Return reference to the first element. */
    T &front() /*noexcept*/ { return data[head]; }
    /* Return const reference to the first element. */
    const T &front() const /*noexcept*/ { return data[head]; }

    /* Return the length of the queue. */
    constexpr int length() const /*noexcept*/ { return __n; }
    /* Return whether the queue is full. */
    bool full() const /*noexcept*/ { return dist == length(); }
    /* Return whether the queue contains element. */
    auto size() const /*noexcept*/ { return  dist; }
    /* Return whether the queue is empty. */
    bool empty()const /*noexcept*/ { return !dist; }

};

}

#endif
