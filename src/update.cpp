#include "update.h"

#include <Preferences.h>

#include "log.h"
#include "owl/github.h"

namespace owl::update {

static String repo;

void begin() {
    Preferences p;
    if (p.begin("update", true)) {
        repo = p.getString("project", "");
        p.end();
    }
}

String project() { return repo; }

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
