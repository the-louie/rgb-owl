#pragma once
// When to look for (and install) firmware updates: at boot, then every periodMs of uptime.
// Hardware-independent; unit-tested on host. The caller performs the actions.

#include <stdint.h>

#include "owl/boot_status.h"

namespace owl {

class UpdateScheduler {
public:
    enum class Action { None, HoldWifi, StartCheck, StartInstall, ReleaseWifi };
    enum class Outcome { Pending, UpToDate, Newer, Failed };  // result of the last check

    UpdateScheduler(uint32_t periodMs, uint32_t onlineTimeoutMs) : period_(periodMs), timeout_(onlineTimeoutMs) {}

    void begin(uint32_t nowMs) {
        next_ = nowMs;  // boot check
        step_ = Step::Idle;
        status_ = UpdateStatus::None;
    }

    // enabled: a project and WiFi credentials are configured. installFailed: the installer gave up.
    Action tick(uint32_t nowMs, bool enabled, bool online, Outcome check, bool installFailed) {
        switch (step_) {
            case Step::Idle:
                if (int32_t(nowMs - next_) < 0) return Action::None;
                next_ = nowMs + period_;
                if (!enabled) return Action::None;
                step_ = Step::WaitOnline;
                since_ = nowMs;
                status_ = UpdateStatus::Checking;
                return Action::HoldWifi;
            case Step::WaitOnline:
                if (online) {
                    step_ = Step::Checking;
                    return Action::StartCheck;
                }
                if (nowMs - since_ >= timeout_) return finish(UpdateStatus::Failed);
                return Action::None;
            case Step::Checking:
                if (check == Outcome::Pending) return Action::None;
                if (check == Outcome::Newer) {
                    step_ = Step::Installing;
                    status_ = UpdateStatus::Downloading;
                    return Action::StartInstall;
                }
                return finish(check == Outcome::UpToDate ? UpdateStatus::UpToDate : UpdateStatus::Failed);
            case Step::Installing:
                if (installFailed) return finish(UpdateStatus::Failed);
                return Action::None;  // success restarts the owl
        }
        return Action::None;
    }

    // Starts a check now (app "Check"), unless one is running.
    void checkNow(uint32_t nowMs) {
        if (step_ == Step::Idle) next_ = nowMs;
    }

    UpdateStatus status() const { return status_; }
    bool busy() const { return step_ != Step::Idle; }

private:
    enum class Step { Idle, WaitOnline, Checking, Installing };

    Action finish(UpdateStatus s) {
        step_ = Step::Idle;
        status_ = s;
        return Action::ReleaseWifi;
    }

    uint32_t period_;
    uint32_t timeout_;
    uint32_t next_ = 0;
    uint32_t since_ = 0;
    Step step_ = Step::Idle;
    UpdateStatus status_ = UpdateStatus::None;
};

}  // namespace owl
