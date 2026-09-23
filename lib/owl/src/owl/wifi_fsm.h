#pragma once
// WiFi connection policy, hardware-independent (unit-tested on host):
//   Connecting --(connected)--> Online --(lost)--> Connecting
//   Connecting --(no link for connectMs)--> Portal (setup AP + captive portal)
//   Portal --(connected)--> Online;  Portal --(portalMs elapsed)--> Connecting (retry home WiFi)
// A home network that is down at boot therefore never leaves the owl stuck in setup mode.

#include <stdint.h>

namespace owl {

class WifiFsm {
public:
    enum class State { Connecting, Online, Portal };
    enum class Action { None, StartPortal, StopPortal, WentOnline, Retry };

    WifiFsm(uint32_t connectMs, uint32_t portalMs) : connectMs_(connectMs), portalMs_(portalMs) {}

    void begin(uint32_t nowMs) {
        state_ = State::Connecting;
        since_ = nowMs;
    }

    State state() const { return state_; }

    Action update(uint32_t nowMs, bool connected) {
        switch (state_) {
            case State::Connecting:
                if (connected) return enter(State::Online, nowMs, Action::WentOnline);
                if (nowMs - since_ >= connectMs_) return enter(State::Portal, nowMs, Action::StartPortal);
                return Action::None;
            case State::Online:
                if (!connected) return enter(State::Connecting, nowMs, Action::None);
                return Action::None;
            case State::Portal:
                if (connected) return enter(State::Online, nowMs, Action::StopPortal);
                if (nowMs - since_ >= portalMs_) return enter(State::Connecting, nowMs, Action::Retry);
                return Action::None;
        }
        return Action::None;
    }

private:
    Action enter(State s, uint32_t nowMs, Action a) {
        state_ = s;
        since_ = nowMs;
        return a;
    }

    uint32_t connectMs_;
    uint32_t portalMs_;
    State state_ = State::Connecting;
    uint32_t since_ = 0;
};

}  // namespace owl
