#pragma once

#include "either.hpp"

namespace ben {

template <typename left_type, typename right_type>
either<left_type, right_type>::either(const left_type& input) : left_(true), lt_(input) {

}

template <typename left_type, typename right_type>
either<left_type, right_type>::either(const right_type& input) : left_(false), rt_(input) {

}

template <typename left_type, typename right_type>
either<left_type, right_type>::either(left_type&& input) : left_(true), lt_(std::move(input)) {

}

template <typename left_type, typename right_type>
either<left_type, right_type>::either(right_type&& input) : left_(false), rt_(std::move(input)) {

}

template <typename left_type, typename right_type>
either<left_type, right_type>& either<left_type, right_type>::operator=(const left_type& input) {
    *this = either(input);
    return *this;
}

template <typename left_type, typename right_type>
either<left_type, right_type>& either<left_type, right_type>::operator=(const right_type& input) {
    *this = either(input);
    return *this;
}

template <typename left_type, typename right_type>
either<left_type, right_type>& either<left_type, right_type>::operator=(left_type&& input) {
    *this = either(std::move(input));
    return *this;
}

template <typename left_type, typename right_type>
either<left_type, right_type>& either<left_type, right_type>::operator=(right_type&& input) {
    *this = either(std::move(input));
    return *this;
}

template <typename left_type, typename right_type>
either<left_type, right_type>::either(const either& other) : left_(other.left_) {
    if (other.is_left()) {
        new (&lt_) left_type(other.lt_);
    } else {
        new (&rt_) right_type(other.rt_);
    }
}

template <typename left_type, typename right_type>
either<left_type, right_type>& either<left_type, right_type>::operator=(const either& other) {
    if (this == &other) {
        return *this;
    }
    destruct_self();
    if (other.is_left()) {
        lt_ = other.lt_;
    } else {
        rt_ = other.rt_;
    }
    left_ = other.left_;
    return *this;
}

template <typename left_type, typename right_type>
either<left_type, right_type>::either(either&& other) : left_(other.left_) {
    if (other.is_left()) {
        new (&lt_) left_type(std::move(other.lt_));
    } else {
        new (&rt_) right_type(std::move(other.rt_));
    }
}

template <typename left_type, typename right_type>
either<left_type, right_type>& either<left_type, right_type>::operator=(either&& other) {
    if (this == &other) {
        return *this;
    }
    destruct_self();
    if (other.is_left()) {
        lt_ = std::move(other.lt_);
    } else {
        rt_ = std::move(other.rt_);
    }
    left_ = other.left_;
    return *this;
}

template <typename left_type, typename right_type>
const left_type& either<left_type, right_type>::as_left() const {
    return lt_;
}

template <typename left_type, typename right_type>
const right_type& either<left_type, right_type>::as_right() const {
    return rt_;
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
    return lt_;
}

template <typename left_type, typename right_type>
right_type& either<left_type, right_type>::right_ref() {
    return rt_;
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
