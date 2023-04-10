#pragma once

#include <cstdio>

namespace ft {

template <typename T>
class Shared_ptr {
public:

    explicit Shared_ptr(T *ptr) : m_ptr(ptr) {
        m_counter = new unsigned int(1);
    }

    explicit Shared_ptr() : m_ptr(NULL) {
        m_counter = new unsigned int(0);
    }

    ~Shared_ptr() {
        if (release()) {
            delete m_ptr;
            delete m_counter;
        }
    }

    Shared_ptr(const Shared_ptr<T> &other) {
        m_counter = other.m_counter;
        m_ptr = other.m_ptr;
        hold();
    }

    template <typename U>
    Shared_ptr(const Shared_ptr<U> &other) {
        m_ptr = other.get();
        m_counter = other.m_counter;
        hold();
    }

    Shared_ptr<T>& operator=(const Shared_ptr<T> &other) {

        if (this == &other)
            return *this;
		if (release())
		{
		  delete m_counter;
		  delete m_ptr;
		}

        m_ptr = other.m_ptr;
        m_counter = other.m_counter;
        hold();
        return *this;
    }

    T * operator->() const {
        return m_ptr;
    }

    T & operator*() const {
        return *m_ptr;
    }

    template <typename U>
    bool operator<(const Shared_ptr<U> &other) {
        return get() < other.get();
    }

    T * get() const {
        return m_ptr;
    }

    void hold() {
        (*m_counter)++;
    }

    bool release() {
        if (*m_counter == 0)
        {
            return true;
        }
        --(*m_counter);
        return *m_counter == 0;
    }

private:
    T *m_ptr;
     unsigned int* m_counter;
};

} 
