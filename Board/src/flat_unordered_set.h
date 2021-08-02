#pragma once

#include "flat_vector.h"

namespace RemoteChess {
    template<typename T, size_t N>
    class flat_unordered_set : private RemoteChess::flat_vector<T, N> {
        public:
        flat_unordered_set() : flat_vector<T, N>() { };
        
        using RemoteChess::flat_vector<T, N>::size;
        using RemoteChess::flat_vector<T, N>::max_capacity;
        using RemoteChess::flat_vector<T, N>::is_full;
        using RemoteChess::flat_vector<T, N>::is_empty;
        using RemoteChess::flat_vector<T, N>::contains;
        using RemoteChess::flat_vector<T, N>::erase;

        void insert(const T& value) {
            if (flat_vector<T, N>::is_full()) return;
            if (contains(value)) return;

            flat_vector<T, N>::push_back(value);
        }

        typename flat_vector<T, N>::iterator begin() { return iterator(*this, 0); }
        typename flat_vector<T, N>::const_iterator cbegin() const { return const_iterator(*this, 0); }
        typename flat_vector<T, N>::iterator end() { return iterator(*this, flat_vector<T, N>::m_size); }
        typename flat_vector<T, N>::const_iterator cend() const { return const_iterator(*this, m_size); }
    };
}
