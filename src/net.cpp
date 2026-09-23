#include "net.h"

#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiManager.h>

#include "config.h"
#include "log.h"
#include "owl/wifi_fsm.h"

namespace owl::net {

static WiFiManager wm;
static WifiFsm fsm(config::WIFI_CONNECT_MS, config::PORTAL_MS);
static bool mdnsStarted = false;

static void connectHome() {
    WiFi.mode(WIFI_STA);
    WiFi.begin();  // credentials stored by the portal (ESP32 WiFi NVS)
}

static void wentOnline() {
    log::printf("wifi: online %s as %s.local", WiFi.localIP().toString().c_str(),
                  config::HOSTNAME);
    if (!mdnsStarted && MDNS.begin(config::HOSTNAME)) {
        MDNS.addService("http", "tcp", 80);
        mdnsStarted = true;
    }
}

void begin() {
    WiFi.setHostname(config::HOSTNAME);
    WiFi.setAutoReconnect(true);
    wm.setConfigPortalBlocking(false);
    wm.setHostname(config::HOSTNAME);
    wm.setConfigPortalTimeout(0);  // lifetime is handled by WifiFsm
    connectHome();
    fsm.begin(millis());
}

void loop() {
    wm.process();
    WifiFsm::State lastState = fsm.state();
    switch (fsm.update(millis(), WiFi.status() == WL_CONNECTED)) {
        case WifiFsm::Action::StartPortal:
            log::printf("wifi: no home network, portal %s", config::SETUP_AP);
            wm.startConfigPortal(config::SETUP_AP);
            break;
        case WifiFsm::Action::StopPortal:
            wm.stopConfigPortal();
            wentOnline();
            break;
        case WifiFsm::Action::WentOnline:
            wentOnline();
            break;
        case WifiFsm::Action::Retry:
            wm.stopConfigPortal();
            connectHome();
            break;
        case WifiFsm::Action::None:
            if (lastState == WifiFsm::State::Online && fsm.state() == WifiFsm::State::Connecting)
                log::printf("wifi: link lost, reconnecting");
            break;
    }
}

bool online() { return fsm.state() == WifiFsm::State::Online; }

void appendDebug(String& j) {
    static const char* const STATES[] = {"connecting", "online", "portal"};
    char buf[200];
    snprintf(buf, sizeof(buf), "\"wifi_state\":\"%s\",\"ssid\":\"%s\",\"rssi\":%d,\"ip\":\"%s\"",
             STATES[int(fsm.state())], WiFi.SSID().c_str(), int(WiFi.RSSI()),
             WiFi.localIP().toString().c_str());
    j += buf;
}

}  // namespace owl::net
