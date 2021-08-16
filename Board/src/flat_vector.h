#pragma once

#include "stddef.h"
#include <algorithm>
#include <iterator>

namespace RemoteChess {
    template<typename T, size_t N>
    class flat_vector {
        T data[N];
        
        protected:
        size_t m_size;

        public:
        using value_type = T;
        using size_type = size_t;

        flat_vector() : m_size(0) { }

        flat_vector(const T copy[N]) : m_size(N) {
            for (m_size i = 0; i < N; i++) {
                data[i] = copy[i];
            }
        } 

        size_t size() const { return m_size; }
        size_t max_capacity() const { return N; }
        bool is_full() const { return m_size == N; }
        bool is_empty() const { return m_size == 0; }

        T& operator[](size_t index) { return data[index]; };
        const T& operator[](size_t index) const { return data[index]; };

        // Does not check for buffer overflow
        void push_back(const T& o) {
            new (&(data[m_size])) T(o); // Placement new over the array
            m_size++;
        }

        // Checks for buffer overflow
        void safe_push_back(const T& o) {
            if (!is_full()) {
                push_back(o);
            }
        }

        // Does not check for buffer underflow
        void pop_back() {
            data[m_size - 1].~T();
            m_size--;
        }

        class iterator {
            size_t index;
            flat_vector<T, N>* parent;

            public:
            iterator(flat_vector<T, N>& parent, size_t index) : parent(&parent), index(index) { }

            bool operator==(const iterator& rhs) const { return index == rhs.index; }
            bool operator!=(const iterator& rhs) const { return !(*this == rhs); }

            iterator& operator++() { index++; return *this; }
            iterator operator++(int) { iterator temp = *this; ++*this; return temp; }
            iterator& operator--() { index--; return *this; }
            iterator operator--(int) { iterator temp = *this; --*this; return temp; }

            T& operator*() const { return (*parent)[index]; }
            T* operator->() const { return &(*parent)[index]; }
        };

        class const_iterator {
            size_t index;
            const flat_vector<T, N>* parent;

            public:
            const_iterator(const flat_vector<T, N>& parent, size_t index) : parent(&parent), index(index) { }

            bool operator==(const const_iterator& rhs) const { return index == rhs.index; }
            bool operator!=(const const_iterator& rhs) const { return !(*this == rhs); }

            const_iterator& operator++() { index++; return *this; }
            const_iterator operator++(int) { const_iterator temp = *this; ++*this; return temp; }
            const_iterator& operator--() { index--; return *this; }
            const_iterator operator--(int) { const_iterator temp = *this; --*this; return temp; }

            const T& operator*() { return (*parent)[index]; }
            const T* operator->() { return &(*parent)[index]; }
        };

        iterator begin() { return iterator(*this, 0); }
        const_iterator begin() const { return const_iterator(*this, 0); }
        const_iterator cbegin() const { return const_iterator(*this, 0); }
        iterator end() { return iterator(*this, m_size); }
        const_iterator end() const { return const_iterator(*this, m_size); }
        const_iterator cend() const { return const_iterator(*this, m_size); }

		iterator find(T& search) {
			auto it = begin();

			for (; it != end(); it++) {
				if (*it == search) {
					return it;
				}
			}

			return it;
		}

        iterator find(const T& search) {
        	auto it = begin();

            for (; it != end(); it++) {
                if (*it == search) {
                    return it;
                }
            }

            return it;
        }

        const_iterator find(const T& search) const {
        	auto it = cbegin();

            for (; it != cend(); it++) {
                if (*it == search) {
                    return it;
                }
            }

            return it;
        }

        bool contains(const T& search) const {
            return find(search) != cend();
        }

        private:
        void swap(T& lhs, T& rhs) {
        	T temp = lhs;
        	lhs = rhs;
        	rhs = temp;
        }

        public:
        // Erases the first instance of the item
        void erase(const T& search) {
            auto it = find(search);

            if (it != end()) {
                for (; it != --end(); it++) {
                    swap(*it, *++iterator(it));
                }

                it->~T();
                m_size--;
            }
        }

        // Erases the first instance of the item
        void erase(T& search) {
            auto it = find(search);

            if (it != end()) {
                for (; it != --end(); it++) {
                    swap(*it, *++iterator(it));
                }

                it->~T();
                m_size--;
            }
        }

        void clear() {
            for (auto it = begin(); it != end(); it++) {
                it->~T();
            }

            m_size = 0;
        }
    };
}
