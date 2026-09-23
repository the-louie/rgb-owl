#pragma once
// Fixed-size text ring buffer: keeps the newest N bytes of log output.
// Hardware-independent; unit-tested on host.

#include <stddef.h>

namespace owl {

template <size_t N>
class RingLog {
public:
    void append(const char* s, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            buf_[head_] = s[i];
            head_ = (head_ + 1) % N;
            if (size_ < N) ++size_;
        }
    }

    size_t size() const { return size_; }

    // Copies the buffered text, oldest first; out must hold size() bytes.
    // If the buffer wrapped, output starts at the first complete line.
    size_t copy(char* out) const {
        size_t start = (head_ + N - size_) % N;
        size_t skip = 0;
        if (size_ == N) {
            while (skip < size_ && buf_[(start + skip) % N] != '\n') ++skip;
            if (skip < size_) ++skip;
        }
        size_t n = 0;
        for (size_t i = skip; i < size_; ++i) out[n++] = buf_[(start + i) % N];
        return n;
    }

private:
    char buf_[N];
    size_t head_ = 0;
    size_t size_ = 0;
};

}  // namespace owl
