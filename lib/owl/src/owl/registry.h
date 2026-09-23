#pragma once
// Fixed list of named items (effects), with lookup and cyclic stepping.
// Hardware-independent; unit-tested on host.

#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace owl {

template <typename T>
class Registry {
public:
    template <size_t N>
    constexpr explicit Registry(T* const (&items)[N]) : items_(items), size_(N) {}

    constexpr size_t size() const { return size_; }
    T& operator[](size_t i) const { return *items_[i]; }

    // Index of the item whose name() equals name, or -1.
    int find(const char* name) const {
        for (size_t i = 0; i < size_; ++i)
            if (strcmp(items_[i]->name(), name) == 0) return int(i);
        return -1;
    }

    size_t next(size_t i) const { return (i + 1) % size_; }

private:
    T* const* items_;
    size_t size_;
};

// Effect time base: advances by frame delta scaled by speed (128 = 1x, 255 ~ 2x, 0 = paused).
class EffectClock {
public:
    uint32_t advance(uint32_t dtMs, uint8_t speed) {
        acc_ += dtMs * speed;
        t_ += acc_ / 128;
        acc_ %= 128;
        return t_;
    }
    uint32_t now() const { return t_; }

private:
    uint32_t t_ = 0;
    uint32_t acc_ = 0;
};

}  // namespace owl
