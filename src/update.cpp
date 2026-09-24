#include "update.h"

#include <Preferences.h>

#include "installer.h"
#include "log.h"
#include "rollback.h"
#include "owl/protocol.h"
#include "owl/github.h"

namespace owl::update {

static String repo;
static void (*sink)(const char*) = nullptr;
static installer::State lastState = installer::State::Idle;
static uint8_t lastPct = 255;

static void emit(const char* state, int pct, const char* msg) {
    char buf[160];
    JsonWriter w(buf, sizeof(buf));
    w.str("type", "update").str("state", state);
    if (pct >= 0) w.num("pct", pct);
    if (msg) w.str("msg", msg);
    const char* j = w.finish();
    if (sink && j) sink(j);
}

void begin() {
    Preferences p;
    if (p.begin("update", true)) {
        repo = p.getString("project", "");
        p.end();
    }
}

String project() { return repo; }

void setEventSink(void (*s)(const char*)) { sink = s; }
bool installing() {
    auto s = installer::state();
    return s == installer::State::Downloading || s == installer::State::Verifying || s == installer::State::Done;
}
uint8_t installPercent() { return installer::percent(); }

bool installFrom(const String& imageUrl, const String& sigUrl) {
    if (!installer::start(imageUrl, sigUrl)) return false;
    log::printf("update: installing %s", imageUrl.c_str());
    lastPct = 255;
    return true;
}

void loop() {
    using S = installer::State;
    S s = installer::state();
    uint8_t p = installer::percent();
    if (s == S::Downloading && (p / 5 != lastPct / 5 || lastPct == 255)) {  // every 5 %
        emit("downloading", p, nullptr);
        lastPct = p;
    }
    if (s == lastState) return;
    lastState = s;
    if (s == S::Verifying) emit("verifying", 100, nullptr);
    if (s == S::Failed) {
        log::printf("update: failed: %s", installer::error());
        emit("failed", -1, installer::error());
        installer::reset();
        lastState = S::Idle;
    }
    if (s == S::Done) {
        log::printf("update: installed, restarting");
        emit("installed", 100, nullptr);
        rollback::expectNewImage();
        delay(500);  // let the event go out
        ESP.restart();
    }
}

bool setProject(const char* in) {
    char norm[100];
    if (*in && !parseGithubProject(in, norm, sizeof(norm))) return false;
    if (!*in) norm[0] = '\0';  // empty clears it
    Preferences p;
    if (p.begin("update", false)) {
        p.putString("project", norm);
        p.end();
    }
    repo = norm;
    log::printf("update: project %s", *norm ? norm : "(none)");
    return true;
}

}  // namespace owl::update
