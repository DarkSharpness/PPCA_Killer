#ifndef _RISC_V_QUEUE_H_
#define _RISC_V_QUEUE_H_

namespace dark {

template <class T,int __n>
struct round_queue {
    T data[__n];
    int head = 0;
    int tail = 0;

    /* Clear all the elements in the queue. */
    void clear() noexcept { head = tail = 0; }

    /* Force to push an element to the back of the queue. */
    void push(const T &__v) noexcept {
        data[tail++] = __v;
        if(tail == __n)  tail =  0;
        if(tail == head) tail = -1;
    }

    /* Pop an element from the front of the queue. */
    void pop() noexcept {
        if(tail == -1)   tail = head;
        ++head;
        if(head == __n)  head = 0;
    }

    T &operator [](int x) noexcept { return data[x]; }

    /* Return reference to the first element. */
    T &front() noexcept { return data[head]; }
    /* Return const reference to the first element. */
    const T &front() const noexcept { return data[head]; }

    /* Return whether the queue is full. */
    bool full() const noexcept { return tail == -1  ; }
    /* Return whether the queue contains element. */
    bool size() const noexcept { return head != tail; }
    /* Return whether the queue is empty. */
    bool empty()const noexcept { return head == tail; }

    constexpr int length() const noexcept { return __n; }
};

}

#endif
