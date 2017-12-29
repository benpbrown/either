#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "either.hpp"
#include "lest.hpp"

#define CASE(name) lest_CASE(specification, name)

static lest::tests specification;

template <typename L, typename R>
void compiles_impl() {
    ben::either<L, R> e1(L{});
    ben::either<L, R> e2(R{});
}

CASE("compiles") {
    using large = std::array<int, 500>;
    using small = std::array<int, 256>;
    compiles_impl<large, small>();
    compiles_impl<small, large>();
    EXPECT(sizeof(ben::either<large, small>) < sizeof(large) + sizeof(small));
}

CASE("basic") {
    auto get_either = [](const std::string& input, bool is_string) -> ben::either<std::string, std::vector<char>> {
        if (is_string) {
            return std::string(input);
        } else {
            std::vector<char> ret;
            for (const auto c : std::string(input)) {
                ret.push_back(c);
            }
            return ret;
        }
    };
    {
        auto str = get_either("a string", true);
        EXPECT(str.is_left());
        EXPECT_NOT(str.is_right());
        const std::string& out = str.as_left();
        EXPECT(out == "a string");
    }
    {
        auto vec = get_either("a vector", false);
        EXPECT(vec.is_right());
        EXPECT_NOT(vec.is_left());
        const std::string expected("a vector");
        const std::vector<char>& out = vec.as_right();
        EXPECT(out.size() == expected.size());
        for (size_t i = 0; i < out.size(); i++) {
            EXPECT(out.at(i) == expected.at(i));
        }
    }
}

namespace {

struct captured_destructable {
    captured_destructable(bool* in) : fired_(in) {}
    ~captured_destructable() {
        if (fired_ != nullptr) {
            *fired_ = true;
        }
    }
    captured_destructable(const captured_destructable&) = delete;
    captured_destructable& operator=(const captured_destructable&) = delete;

    captured_destructable(captured_destructable&& other) : fired_(other.fired_) {
        other.fired_ = nullptr;
    }
    captured_destructable& operator=(captured_destructable&& other) {
        if (this == &other) {
            return *this;
        }
        fired_ = other.fired_;
        other.fired_ = nullptr;
        return *this;
    }

private:
    bool* fired_;
};

} // namespace

CASE("destructor fires") {
    using L = captured_destructable;
    using R = bool;
    {
        bool b = false;
        {
            ben::either<L, R> e(captured_destructable{&b});
        }
        EXPECT(b);
    }
    {
        bool b = false;
        {
            ben::either<R, L> e(captured_destructable{&b});
        }
        EXPECT(b);
    }
}

CASE("constructable with =") {
    using L = int;
    using R = std::string;
    {
        ben::either<L, R> e = 2;
        EXPECT(e.is_left());
        EXPECT(e.as_left() == 2);
    }
    {
        std::string input("hello");
        ben::either<L, R> e = input;
        EXPECT(e.is_right());
        EXPECT(e.as_right() == input);
    }
    {
        ben::either<R, L> e = 2;
        EXPECT(e.is_right());
        EXPECT(e.as_right() == 2);
    }
    {
        std::string input("hello");
        ben::either<R, L> e = input;
        EXPECT(e.is_left());
        EXPECT(e.as_left() == input);
    }
}

CASE("type can change") {
    {
        ben::either<int, captured_destructable> e = 22;
        EXPECT(e.is_left());
        EXPECT(e.as_left() == 22);
        bool b = false;
        e = captured_destructable{&b};
        EXPECT(e.is_right());
        e = 55;
        EXPECT(b == true);
        EXPECT(e.is_left());
        EXPECT(e.as_left() == 55);
    }
    {
        ben::either<captured_destructable, int> e = 22;
        EXPECT(e.is_right());
        EXPECT(e.as_right() == 22);
        bool b = false;
        e = captured_destructable{&b};
        EXPECT(e.is_left());
        e = 55;
        EXPECT(b == true);
        EXPECT(e.is_right());
        EXPECT(e.as_right() == 55);
    }
}

CASE("differing type sizes") {
    ben::either<uint8_t, uint64_t> nums = uint64_t{0xffffffffff};
    EXPECT(nums.is_right());
    EXPECT(nums.as_right() == 0xffffffffff);
    nums = uint8_t{2};
    EXPECT(nums.is_left());
    EXPECT(nums.as_left() == 2);
    nums = uint64_t{16};
    EXPECT(nums.is_right());
    EXPECT(nums.as_right() == 16);
}

CASE("operator equal") {
    ben::either<int, char> e(2);
    ben::either<int, char> f('c');
    EXPECT_NOT(e == f);
    e = 'c';
    EXPECT(e == f);
    f = 2;
    EXPECT_NOT(e == f);
    e = 4;
    f = 2;
    EXPECT_NOT(e == f);
    e = 'c';
    f = 'f';
    EXPECT_NOT(e == f);
}

CASE("slow path and fast path") {
    constexpr size_t size = 10;
    using slow_t = std::unique_ptr<char[]>;
    using fast_t = std::array<char, size>;
    {
        std::string expected;
        ben::either<slow_t, fast_t> foo(std::make_unique<char[]>(size));
        EXPECT(foo.is_left());
        char* val = foo.as_left().get();
        for (size_t i = 0; i < size - 1; i++) {
            expected += 'a';
            val[i] = 'a';
        }
        val[size - 1] = '\0';
        EXPECT(foo.as_left().get() == expected);
        foo = fast_t{};
        EXPECT(foo.is_right());
        for (size_t i = 0; i < size; i++) {
            char* d = foo.right_ref().data(); 
            d[i] = 'q'; 
        }
        EXPECT(std::string(foo.as_right().begin(), foo.as_right().end()) == "qqqqqqqqqq");
    }
    {
        std::string expected;
        ben::either<fast_t, slow_t> foo(std::make_unique<char[]>(size));
        EXPECT(foo.is_right());
        char* val = foo.as_right().get();
        for (size_t i = 0; i < size - 1; i++) {
            expected += 'a';
            val[i] = 'a';
        }
        val[size - 1] = '\0';
        EXPECT(foo.as_right().get() == expected);
        foo = fast_t{};
        EXPECT(foo.is_left());
        for (size_t i = 0; i < size; i++) {
            char* d = foo.left_ref().data(); 
            d[i] = 'q'; 
        }
        EXPECT(std::string(foo.as_left().begin(), foo.as_left().end()) == "qqqqqqqqqq");
    }
}

CASE("move constructor regression") {
    constexpr size_t size = 10;
    using slow_t = std::unique_ptr<char[]>;
    using fast_t = std::array<char, size>;

    ben::either<slow_t, fast_t> foo(std::make_unique<char[]>(size));
    EXPECT(foo.is_left());
    const char* ptr = foo.as_left().get();
    const bool equal = ptr == nullptr;
    EXPECT_NOT(equal);

    ben::either<slow_t, fast_t> quux(std::move(foo));
    EXPECT(foo.as_left().get() == nullptr);
    EXPECT(ptr == quux.as_left().get());
}

struct only_movable {
    only_movable(int* counter) : counter_(counter) {}
    ~only_movable() {
        if (counter_) {
            (*counter_)++;
        }
    }
    only_movable(const only_movable& other) = delete;
    only_movable(only_movable&& other) : counter_(other.counter_) {
        other.counter_ = nullptr;
    }
    only_movable& operator=(const only_movable& other) = delete;
    only_movable& operator=(only_movable&& other) {
        int* c = other.counter_;
        other.counter_ = nullptr;
        counter_ = c;
        return *this;
    }
private:
    int* counter_;
};

CASE("move only") {
    int c = 0;
    {
        constexpr size_t size = 10;
        using slow_t = only_movable;
        using fast_t = std::array<char, size>;

        ben::either<slow_t, fast_t> foo{only_movable{&c}};
        EXPECT(foo.is_left());
        EXPECT(c == 0);

        ben::either<slow_t, fast_t> quux(std::move(foo));
    }
    EXPECT(c == 1);
}

CASE("move constructor 2") {
    bool b = false;
    ben::either<captured_destructable, int> e = captured_destructable{&b};
    EXPECT_NOT(b);
    ben::either<captured_destructable, int> f = 22;
    e = std::move(f);
    EXPECT(b);
}

CASE("move constructor 3") {
    bool b = false;
    ben::either<captured_destructable, int> e = 22;
    ben::either<captured_destructable, int> f = captured_destructable{&b};
    e = std::move(f);
    EXPECT_NOT(b);
}

CASE("left type constructor") {
    int input = 2;
    ben::either<int, char> e(input);
    EXPECT(e.is_left());
    EXPECT(e.as_left() == input);
}

CASE("right type constructor") {
    char input = 'c';
    ben::either<int, char> e(input);
    EXPECT(e.is_right());
    EXPECT(e.as_right() == input);
}

CASE("left type move constructor") {
    int input = 2;
    ben::either<int, char> e(std::move(input));
    EXPECT(e.is_left());
    EXPECT(e.as_left() == 2);
}

CASE("right type move constructor") {
    char input = 'c';
    ben::either<int, char> e(std::move(input));
    EXPECT(e.is_right());
    EXPECT(e.as_right() == 'c');
}

CASE("left type operator=") {
    int input = 2;
    ben::either<int, char> e = input;
    EXPECT(e.is_left());
    EXPECT(e.as_left() == input);
}

CASE("left type operator= with overwrite") {
    bool b = false;
    ben::either<captured_destructable, int> e(captured_destructable{&b});
    EXPECT_NOT(b);
    EXPECT(e.is_left());
    int overwrite = 22;
    e = overwrite;
    EXPECT(b);
    EXPECT(e.is_right());
    EXPECT(e.as_right() == 22);
}

CASE("right type operator=") {
    char input = 'c';
    ben::either<int, char> e = input;
    EXPECT(e.is_right());
    EXPECT(e.as_right() == input);
}

CASE("right type operator= with overwrite") {
    bool b = false;
    ben::either<int, captured_destructable> e(captured_destructable{&b});
    EXPECT_NOT(b);
    EXPECT(e.is_right());
    int overwrite = 22;
    e = overwrite;
    EXPECT(b);
    EXPECT(e.is_left());
    EXPECT(e.as_left() == 22);
}

CASE("left type move operator=") {
    int input = 2;
    ben::either<int, char> e = std::move(input);
    EXPECT(e.is_left());
    EXPECT(e.as_left() == 2);
}

CASE("left type move operator= with overwrite") {
    bool b = false;
    ben::either<captured_destructable, int> e(captured_destructable{&b});
    EXPECT_NOT(b);
    EXPECT(e.is_left());
    int overwrite = 22;
    e = std::move(overwrite);
    EXPECT(b);
    EXPECT(e.is_right());
    EXPECT(e.as_right() == 22);
}

CASE("right type move operator=") {
    char input = 'c';
    ben::either<int, char> e = std::move(input);
    EXPECT(e.is_right());
    EXPECT(e.as_right() == 'c');
}

CASE("right type move operator= with overwrite") {
    bool b = false;
    ben::either<int, captured_destructable> e(captured_destructable{&b});
    EXPECT_NOT(b);
    EXPECT(e.is_right());
    int overwrite = 22;
    e = std::move(overwrite);
    EXPECT(b);
    EXPECT(e.is_left());
    EXPECT(e.as_left() == 22);
}

CASE("copy constructor") {
    ben::either<std::string, int> e(std::string("hello"));
    ben::either<std::string, int> f(e);
    EXPECT(e.is_left());
    EXPECT(f.is_left());
    const bool equal = e.as_left().c_str() == f.as_left().c_str();
    EXPECT_NOT(equal); // memory addr different
    EXPECT(e.as_left() == f.as_left()); // contents the same
}

CASE("copy assignment") {
    ben::either<std::string, int> e(std::string("hello"));
    ben::either<std::string, int> f = e;
    EXPECT(e.is_left());
    EXPECT(f.is_left());
    const bool equal = e.as_left().c_str() == f.as_left().c_str();
    EXPECT_NOT(equal); // memory addr different
    EXPECT(e.as_left() == f.as_left()); // contents the same
}

namespace {

struct destruct_counter {
    destruct_counter(int* counter) : counter_(counter) {}
    ~destruct_counter() {
        if (counter_) {
            (*counter_)++;
        }
    }

    destruct_counter(const destruct_counter& other) : counter_(other.counter_) {

    }

    destruct_counter(destruct_counter&& other) : counter_(other.counter_) {
        other.counter_ = nullptr;
    }

    destruct_counter& operator=(const destruct_counter& other) {
        counter_ = other.counter_;
        return *this;
    }

    destruct_counter& operator=(destruct_counter&& other) {
        if (this == &other) {
            return *this;
        }
        counter_ = other.counter_;
        other.counter_ = nullptr;
        return *this;
    }

private:
    int* counter_;
};

}; // namespace

CASE("copy assignment with overwrite") {
    int c = 0;
    {
        ben::either<destruct_counter, int> e(destruct_counter{&c});
        EXPECT(c == 0);
        EXPECT(e.is_left());
        ben::either<destruct_counter, int> f = 12;
        EXPECT(f.is_right());
        f = e;
        EXPECT(c == 0);
        ben::either<destruct_counter, int> overwrite = 24;
        e = overwrite;
        EXPECT(c == 1);
        EXPECT(e.is_right());
        EXPECT(e.as_right() == 24);
        f = overwrite;
        EXPECT(c == 2);
        EXPECT(f.is_right());
        EXPECT(f.as_right() == 24);
    }
    EXPECT(c == 2);
}

CASE("move constructor again") {
    bool b = false;
    ben::either<captured_destructable, int> e(captured_destructable{&b});
    EXPECT_NOT(b);
    ben::either<captured_destructable, int> f(std::move(e));
    EXPECT_NOT(b);
    EXPECT(f.is_left());
}

CASE("move assignment") {
    bool b = false;
    ben::either<captured_destructable, int> e(captured_destructable{&b});
    EXPECT_NOT(b);
    ben::either<captured_destructable, int> f = std::move(e);
    EXPECT_NOT(b);
    EXPECT(f.is_left());
}

CASE("move assignment with overwrite") {
    int c = 0;
    {
        ben::either<destruct_counter, int> e(destruct_counter{&c});
        EXPECT(c == 0);
        EXPECT(e.is_left());
        ben::either<destruct_counter, int> f = 12;
        EXPECT(f.is_right());
        f = std::move(e);
        EXPECT(c == 0);
        ben::either<destruct_counter, int> overwrite = 24;
        e = std::move(overwrite);
        EXPECT(c == 0);
        EXPECT(e.is_right());
        EXPECT(e.as_right() == 24);
        f = std::move(e);
        EXPECT(c == 1);
        EXPECT(f.is_right());
        EXPECT(f.as_right() == 24);
    }
    EXPECT(c == 1);
}

class some_non_pod {
public:
    some_non_pod() : val_(3) {
        assert(val_ != 0);
    }
    some_non_pod(int val) : val_(val) {
        assert(val_ != 0);
    }
    ~some_non_pod() {
        assert(val_ != 0);
    }
    some_non_pod(const some_non_pod& other) : val_(other.val_) {
        assert(val_ != 0);
    }
    some_non_pod(some_non_pod&& other) : val_(other.val_) {
        assert(val_ != 0);
        other.val_ = -1;
    }
    some_non_pod& operator=(const some_non_pod& other) {
        assert(val_ != 0);
        val_ = other.val_;
        return *this;
    }

    some_non_pod& operator=(some_non_pod&& other) {
        if (this == &other) {
            return *this;
        }
        assert(val_ != 0);
        val_ = other.val_;
        other.val_ = -1;
        return *this;
    }

private:
    int val_;
};

CASE("test invalid zero state") {
    constexpr size_t size = 10;
    using slow_t = some_non_pod;
    using fast_t = std::array<char, size>;

    ben::either<slow_t, fast_t> foo(some_non_pod{2});
    EXPECT(foo.is_left());

    ben::either<slow_t, fast_t> quux(std::move(foo));
}

int main(int argc, char* argv[]) {
    return lest::run(specification, argc, argv);
}
