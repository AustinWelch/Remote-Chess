#pragma once

namespace RemoteChess {
    template<typename T>
    class optional {
        union {
            char empty;
            T value;
        };

        bool hasValue;

        public:
        optional() : empty(0), hasValue(false) { }
        optional(const T& v) : value(v), hasValue(true) { }
        optional(std::nullptr_t) : optional() { }

        optional(const optional<T>& copy) : hasValue(copy.hasValue) {
            if (copy.hasValue) {
                value = copy.value;
            } else {
                empty = 0;
            }
        }

        optional<T>& operator=(const optional<T>& copy) {
            if (copy.hasValue) {
                value = copy.value;
                hasValue = true;
            } else {
                if (hasValue)
                    value.~T();

                empty = 0;
                hasValue = false;
            }

            return *this;
        }

        optional<T>& operator=(std::nullptr_t) {
            if (hasValue)
                value.~T();

            empty = 0;
            hasValue = false;

            return *this;
        }

        operator bool() const {
            return hasValue;
        }

        bool operator==(const optional<T>& rhs) const {
            if (!rhs.hasValue)
                return hasValue == false;
            else
                return hasValue == true && this->value == rhs.value;
        }

        bool operator!=(const optional<T>& rhs) const {
            return !(*this == rhs);
        }

        // bool operator==(const T& rhs) const {
        //     if (hasValue)
        //         return value == rhs;
        //     else
        //         return false;
        // }

        // bool operator!=(const T& rhs) const {
        //     return !(*this == rhs);
        // }

        friend bool operator==(const T& lhs, const optional<T>& rhs) {
            return rhs == lhs;
        }

        friend bool operator!=(const T& lhs, const optional<T>& rhs) {
            return rhs != lhs;
        }

        bool HasValue() const {
            return hasValue;
        }

        T& Value() {
            return value;
        }

        const T& Value() const {
            return value;
        }

        T& operator*() {
            return value;
        }

        const T& operator*() const {
            return value;
        }

        T* operator->() {
            return &value;
        }

        const T* operator->() const {
            return &value;
        }
    };
}
