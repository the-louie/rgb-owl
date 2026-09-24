#pragma once
// BLE command protocol + JSON output helpers. Hardware-independent; unit-tested on host.
//
// Command line (written to the `command` characteristic):
//     verb [key=value&key=value...]      keys/values URL-encoded (%XX, '+' = space)
// e.g. "set brightness=80&effect=flame", "wifi_test ssid=My%20Net&pass=s3cr%26t"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace owl {

class Command {
public:
    static constexpr size_t MAX_LEN = 511;
    static constexpr size_t MAX_PAIRS = 12;

    // Parses line; returns false (and leaves the command empty) if it is malformed.
    bool parse(const char* line, size_t len) {
        n_ = 0;
        verb_ = buf_;
        buf_[0] = '\0';
        if (len == 0 || len > MAX_LEN) return false;
        memcpy(buf_, line, len);
        buf_[len] = '\0';
        char* sp = strchr(buf_, ' ');
        if (sp) *sp = '\0';
        if (!*buf_) return false;
        for (char* v = buf_; *v; ++v)
            if (!((*v >= 'a' && *v <= 'z') || *v == '_' || (*v >= '0' && *v <= '9'))) return false;
        if (!sp || !sp[1]) return true;
        char* p = sp + 1;
        while (p && *p) {
            char* amp = strchr(p, '&');
            if (amp) *amp = '\0';
            char* eq = strchr(p, '=');
            if (!eq || eq == p || n_ == MAX_PAIRS) return fail();
            *eq = '\0';
            if (!decode(p) || !decode(eq + 1)) return fail();
            pairs_[n_++] = {p, eq + 1};
            p = amp ? amp + 1 : nullptr;
        }
        return true;
    }
    bool parse(const char* line) { return parse(line, strlen(line)); }

    const char* verb() const { return verb_; }
    size_t size() const { return n_; }
    const char* key(size_t i) const { return pairs_[i].key; }
    const char* value(size_t i) const { return pairs_[i].value; }

    // Value for key, or nullptr.
    const char* get(const char* key) const {
        for (size_t i = 0; i < n_; ++i)
            if (!strcmp(pairs_[i].key, key)) return pairs_[i].value;
        return nullptr;
    }

private:
    struct Pair {
        const char* key;
        const char* value;
    };

    bool fail() {
        n_ = 0;
        buf_[0] = '\0';
        return false;
    }

    static int hex(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    }

    // In-place URL decoding; rejects bad escapes and NUL bytes.
    static bool decode(char* s) {
        char* out = s;
        for (char* in = s; *in; ++in) {
            if (*in == '+') {
                *out++ = ' ';
            } else if (*in == '%') {
                int hi = hex(in[1]), lo = hi < 0 ? -1 : hex(in[2]);
                if (hi < 0 || lo < 0 || (hi == 0 && lo == 0)) return false;
                *out++ = char(hi * 16 + lo);
                in += 2;
            } else {
                *out++ = *in;
            }
        }
        *out = '\0';
        return true;
    }

    char buf_[MAX_LEN + 1] = {};
    const char* verb_ = buf_;
    Pair pairs_[MAX_PAIRS] = {};
    size_t n_ = 0;
};

// Minimal JSON object writer into a fixed buffer. ok() is false if anything was truncated.
class JsonWriter {
public:
    JsonWriter(char* buf, size_t cap) : buf_(buf), cap_(cap) { put("{"); }

    JsonWriter& str(const char* key, const char* v) {
        this->key(key);
        quoted(v);
        return *this;
    }
    JsonWriter& num(const char* key, long v) {
        this->key(key);
        char t[24];
        snprintf(t, sizeof(t), "%ld", v);
        put(t);
        return *this;
    }
    JsonWriter& boolean(const char* key, bool v) {
        this->key(key);
        put(v ? "true" : "false");
        return *this;
    }
    // Inserts pre-built JSON (object/array) as the value.
    JsonWriter& raw(const char* key, const char* json) {
        this->key(key);
        put(json);
        return *this;
    }

    // Closes the object; returns the string, or nullptr if it did not fit.
    const char* finish() {
        put("}");
        return ok_ ? buf_ : nullptr;
    }
    bool ok() const { return ok_; }

private:
    void key(const char* k) {
        if (first_) {
            first_ = false;
        } else {
            put(",");
        }
        quoted(k);
        put(":");
    }
    void quoted(const char* s) {
        put("\"");
        for (; *s; ++s) {
            unsigned char c = static_cast<unsigned char>(*s);
            if (c == '"' || c == '\\') {
                char t[3] = {'\\', char(c), 0};
                put(t);
            } else if (c < 0x20) {
                char t[8];
                snprintf(t, sizeof(t), "\\u%04x", c);
                put(t);
            } else {
                char t[2] = {char(c), 0};
                put(t);
            }
        }
        put("\"");
    }
    void put(const char* s) {
        size_t l = strlen(s);
        if (!ok_ || len_ + l + 1 > cap_) {
            ok_ = false;
            return;
        }
        memcpy(buf_ + len_, s, l + 1);
        len_ += l;
    }

    char* buf_;
    size_t cap_;
    size_t len_ = 0;
    bool first_ = true;
    bool ok_ = true;
};

// Semantic version "X.Y.Z[-pre][+build]" (optional leading 'v').
struct SemVer {
    long major = 0, minor = 0, patch = 0;
    char pre[32] = {};  // empty for a release

    bool parse(const char* s) {
        if (*s == 'v') ++s;
        char* end;
        long* parts[] = {&major, &minor, &patch};
        for (int i = 0; i < 3; ++i) {
            if (*s < '0' || *s > '9') return false;
            *parts[i] = strtol(s, &end, 10);
            s = end;
            if (i < 2) {
                if (*s != '.') return false;
                ++s;
            }
        }
        pre[0] = '\0';
        if (*s == '-') {
            ++s;
            size_t n = strcspn(s, "+");
            if (n == 0 || n >= sizeof(pre)) return false;
            memcpy(pre, s, n);
            pre[n] = '\0';
            s += n;
        }
        return *s == '\0' || *s == '+';
    }
};

// <0, 0, >0 like strcmp. A pre-release sorts before its release; pre-release
// identifiers compare dot-separated, numeric ones numerically (SemVer 2.0 §11).
inline int compare(const SemVer& a, const SemVer& b) {
    if (a.major != b.major) return a.major < b.major ? -1 : 1;
    if (a.minor != b.minor) return a.minor < b.minor ? -1 : 1;
    if (a.patch != b.patch) return a.patch < b.patch ? -1 : 1;
    if (!a.pre[0] || !b.pre[0]) return a.pre[0] ? -1 : b.pre[0] ? 1 : 0;
    const char *p = a.pre, *q = b.pre;
    while (*p && *q) {
        size_t lp = strcspn(p, "."), lq = strcspn(q, ".");
        bool np = strspn(p, "0123456789") == lp, nq = strspn(q, "0123456789") == lq;
        int c;
        if (np && nq) {
            long x = strtol(p, nullptr, 10), y = strtol(q, nullptr, 10);
            c = x < y ? -1 : x > y ? 1 : 0;
        } else if (np != nq) {
            c = np ? -1 : 1;
        } else {
            c = strncmp(p, q, lp < lq ? lp : lq);
            if (!c && lp != lq) c = lp < lq ? -1 : 1;
        }
        if (c) return c;
        p += lp + (p[lp] == '.');
        q += lq + (q[lq] == '.');
    }
    return *p ? 1 : *q ? -1 : 0;
}

}  // namespace owl
