#pragma once
// When WiFi is on (SPEC v2 §WiFi). Hardware-independent; unit-tested on host.
//   boot, configured  -> Connecting -(link)-> Online for windowMs -> Off
//   Connecting -(no link for connectMs)-> Off (status Failed)
//   debug mode keeps WiFi on (and turns it on when Off); link loss -> Connecting
//   hold (update checks) also keeps/turns WiFi on, but gives up after connectMs like the boot attempt
// The caller switches the radio on Action::Start / Action::Stop.

#include <stdint.h>

namespace owl {

class WifiPolicy {
public:
    enum class Link { Off, Connecting, Online };
    enum class Status { Unconfigured, Connecting, Connected, Failed, Off };
    enum class Action { None, Start, Stop };

    WifiPolicy(uint32_t connectMs, uint32_t windowMs) : connectMs_(connectMs), windowMs_(windowMs) {}

    Action begin(uint32_t nowMs, bool configured) {
        configured_ = configured;
        if (!configured) {
            status_ = Status::Unconfigured;
            return Action::None;
        }
        return enterConnecting(nowMs, Action::Start);
    }

    void setConfigured(bool c) {
        configured_ = c;
        if (!c && link_ == Link::Off) status_ = Status::Unconfigured;
    }
    void setDevmode(bool on) { devmode_ = on; }
    void setHold(bool on) { hold_ = on; }
    bool hold() const { return hold_; }
    bool devmode() const { return devmode_; }
    Link link() const { return link_; }
    Status status() const { return status_; }
    // True while the post-boot window is open (it opens at the first connection).
    bool windowOpen(uint32_t nowMs) const { return windowSet_ && int32_t(nowMs - windowEnd_) < 0; }

    Action update(uint32_t nowMs, bool connected) {
        switch (link_) {
            case Link::Off:
                if ((devmode_ || hold_) && configured_) return enterConnecting(nowMs, Action::Start);
                return Action::None;
            case Link::Connecting:
                if (connected) {
                    if (!windowSet_) {
                        windowSet_ = true;
                        windowEnd_ = nowMs + windowMs_;
                    }
                    link_ = Link::Online;
                    status_ = Status::Connected;
                    return Action::None;
                }
                if (nowMs - since_ < connectMs_) return Action::None;
                if (devmode_) {
                    since_ = nowMs;  // keep trying while debugging
                    status_ = Status::Failed;
                    return Action::None;
                }
                windowSet_ = true;  // a failed boot attempt closes the window
                windowEnd_ = nowMs;
                return enterOff(Status::Failed);
            case Link::Online:
                if (!connected) {
                    link_ = Link::Connecting;
                    since_ = nowMs;
                    status_ = Status::Connecting;
                    return Action::None;
                }
                if (!devmode_ && !hold_ && !windowOpen(nowMs)) return enterOff(Status::Off);
                return Action::None;
        }
        return Action::None;
    }

private:
    Action enterConnecting(uint32_t nowMs, Action a) {
        link_ = Link::Connecting;
        since_ = nowMs;
        status_ = Status::Connecting;
        return a;
    }
    Action enterOff(Status s) {
        link_ = Link::Off;
        status_ = s;
        return Action::Stop;
    }

    uint32_t connectMs_;
    uint32_t windowMs_;
    Link link_ = Link::Off;
    Status status_ = Status::Unconfigured;
    bool configured_ = false;
    bool devmode_ = false;
    bool hold_ = false;
    bool windowSet_ = false;
    uint32_t windowEnd_ = 0;
    uint32_t since_ = 0;
};

}  // namespace owl
