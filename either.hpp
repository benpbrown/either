#pragma once

#include <array>
#include <cstring>

namespace ben {

// either implements a type variant that is either left_type
// or right type. The caller is responsible for making sure
// that if they call methods that return a type (e.g. as_left()),
// that either is actually of that type.
template <typename left_type, typename right_type>
class either {
public:
    ~either();

    either(const left_type& input);
    either(const right_type& input);

    either(left_type&& input);
    either(right_type&& input);

    either& operator=(const left_type& other);
    either& operator=(const right_type& other);

    either& operator=(left_type&& other);
    either& operator=(right_type&& other);

    either(const either& other);
    either& operator=(const either& other);

    either(either&& other);
    either& operator=(either&& other);

    const left_type& as_left() const;
    const right_type& as_right() const;

    left_type& left_ref();
    right_type& right_ref();

    bool is_left() const;
    bool is_right() const;

    bool operator==(const either& other) const;

private:
    template <typename T>
    static void destruct(T& in);
    void destruct_self();

    bool left_ = false;

    union {
        left_type lt_;
        right_type rt_;
    };
};

} // namespace ben

#include "either.ipp"
