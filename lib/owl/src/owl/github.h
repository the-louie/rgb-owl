#pragma once
// GitHub project references for the update source. Hardware-independent; unit-tested on host.

#include <stddef.h>
#include <string.h>

namespace owl {

// Normalises "https://github.com/owner/repo[.git][/...]", "github.com/owner/repo" or
// "owner/repo" to "owner/repo" in out (cap bytes). Returns false if it is not one of those.
inline bool parseGithubProject(const char* in, char* out, size_t cap) {
    const char* prefixes[] = {"https://github.com/", "http://github.com/", "github.com/"};
    for (const char* p : prefixes)
        if (!strncmp(in, p, strlen(p))) {
            in += strlen(p);
            break;
        }
    // GitHub owners: letters, digits, '-'. Repositories may also use '_' and '.'.
    auto ok = [](char c, bool repo) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' ||
               (repo && (c == '_' || c == '.'));
    };
    size_t n = 0, slash = 0;
    const char* s = in;
    for (; *s && *s != '?' && *s != '#'; ++s) {
        if (*s == '/') {
            if (++slash == 2) break;  // ignore anything after owner/repo
            if (n == 0) return false;
        } else if (!ok(*s, slash == 1)) {
            return false;
        }
        if (n + 1 >= cap) return false;
        out[n++] = *s;
    }
    if (slash == 0 || out[n - 1] == '/') return false;
    if (n > 4 && !strncmp(out + n - 4, ".git", 4)) n -= 4;
    out[n] = '\0';
    return true;
}

}  // namespace owl
