#include "log.h"

#include <stdarg.h>

#include "owl/ringlog.h"

namespace owl::log {

static RingLog<4096> ring;

void printf(const char* fmt, ...) {
    char line[256];
    uint32_t ms = millis();
    int n = snprintf(line, sizeof(line), "[%6lu.%03lu] ", (unsigned long)(ms / 1000),
                     (unsigned long)(ms % 1000));
    va_list ap;
    va_start(ap, fmt);
    int m = vsnprintf(line + n, sizeof(line) - n - 1, fmt, ap);
    va_end(ap);
    n += m < 0 ? 0 : min(m, int(sizeof(line)) - n - 2);
    line[n++] = '\n';
    ring.append(line, n);
    Serial.write(reinterpret_cast<const uint8_t*>(line), n - 1);
    Serial.write("\r\n");
}

String dump() {
    static char out[4096];
    size_t n = ring.copy(out);
    String s;
    s.reserve(n);
    s.concat(out, n);
    return s;
}

}  // namespace owl::log
