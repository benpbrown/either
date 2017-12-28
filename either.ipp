#pragma once

#include "either.hpp"

namespace ben {

template <typename left_type, typename right_type>
either<left_type, right_type>::either(const left_type& input) : left_(true) {
    left_ref() = input;
}

template <typename left_type, typename right_type>
either<left_type, right_type>::either(const right_type& input) : left_(false) {
    right_ref() = input;
}

template <typename left_type, typename right_type>
either<left_type, right_type>::either(left_type&& input) : left_(true) {
    left_ref() = std::move(input);
}

template <typename left_type, typename right_type>
either<left_type, right_type>::either(right_type&& input) : left_(false) {
    right_ref() = std::move(input);
}

template <typename left_type, typename right_type>
either<left_type, right_type>& either<left_type, right_type>::operator=(const left_type& other) {
    *this = either(other);
    return *this;
}

template <typename left_type, typename right_type>
either<left_type, right_type>& either<left_type, right_type>::operator=(const right_type& other) {
    *this = either(other);
    return *this;
}

template <typename left_type, typename right_type>
either<left_type, right_type>& either<left_type, right_type>::operator=(left_type&& other) {
    *this = either(std::move(other));
    return *this;
}

template <typename left_type, typename right_type>
either<left_type, right_type>& either<left_type, right_type>::operator=(right_type&& other) {
    *this = either(std::move(other));
    return *this;
}

template <typename left_type, typename right_type>
either<left_type, right_type>::either(const either& other) : left_(other.left_) {
    if (other.is_left()) {
        left_ref() = other.as_left();
    } else {
        right_ref() = other.as_right();
    }
}

template <typename left_type, typename right_type>
either<left_type, right_type>& either<left_type, right_type>::operator=(const either& other) {
    if (this == &other) {
        return *this;
    }
    destruct_self();
    if (other.is_left()) {
        left_ref() = other.as_left();
    } else {
        right_ref() = other.as_right();
    }
    left_ = other.left_;
    return *this;
}

template <typename left_type, typename right_type>
either<left_type, right_type>::either(either&& other) : left_(other.left_) {
    if (other.is_left()) {
        left_ref() = std::move(other.left_ref());
    } else {
        right_ref() = std::move(other.right_ref());
    }
}

template <typename left_type, typename right_type>
either<left_type, right_type>& either<left_type, right_type>::operator=(either&& other) {
    if (this == &other) {
        return *this;
    }
    destruct_self();
    if (other.is_left()) {
        left_ref() = std::move(other.left_ref());
    } else {
        right_ref() = std::move(other.right_ref());
    }
    left_ = other.left_;
    return *this;
}

template <typename left_type, typename right_type>
const left_type& either<left_type, right_type>::as_left() const {
    return *reinterpret_cast<const left_type*>(storage_.data());
}

template <typename left_type, typename right_type>
const right_type& either<left_type, right_type>::as_right() const {
    return *reinterpret_cast<const right_type*>(storage_.data());
}

template <typename left_type, typename right_type>
bool either<left_type, right_type>::is_left() const {
    return left_;
}

template <typename left_type, typename right_type>
bool either<left_type, right_type>::is_right() const {
    return !is_left();
}

template <typename left_type, typename right_type>
either<left_type, right_type>::~either<left_type, right_type>() {
    destruct_self();
}

template <typename left_type, typename right_type>
bool either<left_type, right_type>::operator==(const either& other) const {
	if (is_left()) {
		if (!other.is_left()) {
			return false;
		}
		return as_left() == other.as_left();
	} else { // is_right()
		if (!other.is_right()) {
			return false;
		}
		return as_right() == other.as_right();
	}
}

template <typename left_type, typename right_type>
template <typename T>
void either<left_type, right_type>::destruct(T& in) {
    in.~T();
}

template <typename left_type, typename right_type>
left_type& either<left_type, right_type>::left_ref() {
    return *reinterpret_cast<left_type*>(storage_.data());
}

template <typename left_type, typename right_type>
right_type& either<left_type, right_type>::right_ref() {
    return *reinterpret_cast<right_type*>(storage_.data());
}

template <typename left_type, typename right_type>
void either<left_type, right_type>::destruct_self() {
    if (left_) {
        destruct(left_ref());
    } else {
        destruct(right_ref());
    }
}

} // namespace ben
