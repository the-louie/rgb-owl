#include "net.h"

#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include "config.h"
#include "log.h"

namespace owl::net {

static WifiPolicy policy(config::WIFI_CONNECT_MS, config::WIFI_WINDOW_MS);
static String ssid, pass;
static bool mdnsStarted = false;
static bool wasOnline = false;

static void loadCredentials() {
    Preferences p;
    if (p.begin("wifi", true)) {
        ssid = p.getString("ssid", "");
        pass = p.getString("pass", "");
        p.end();
    }
    if (ssid.length()) return;
    // One-time migration from firmware v1 (WiFiManager kept them in the ESP WiFi config).
    wifi_config_t c = {};
    WiFi.mode(WIFI_STA);
    if (esp_wifi_get_config(WIFI_IF_STA, &c) == ESP_OK && c.sta.ssid[0]) {
        saveCredentials(reinterpret_cast<const char*>(c.sta.ssid), reinterpret_cast<const char*>(c.sta.password));
        log::printf("wifi: migrated credentials for %s", ssid.c_str());
    }
    WiFi.mode(WIFI_OFF);
}

static void radioOn() {
    WiFi.persistent(false);  // credentials live in our NVS, not the ESP WiFi config
    WiFi.setHostname(config::HOSTNAME);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid.c_str(), pass.c_str());
    log::printf("wifi: connecting to %s", ssid.c_str());
}

static void radioOff() {
    if (mdnsStarted) {
        MDNS.end();
        mdnsStarted = false;
    }
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    log::printf("wifi: off (%s)", policy.status() == WifiPolicy::Status::Failed ? "connect failed" : "window closed");
}

void begin() {
    loadCredentials();
    if (policy.begin(millis(), ssid.length() > 0) == WifiPolicy::Action::Start) radioOn();
    else log::printf("wifi: not configured");
}

void loop() {
    bool connected = WiFi.status() == WL_CONNECTED;
    switch (policy.update(millis(), connected)) {
        case WifiPolicy::Action::Start: radioOn(); break;
        case WifiPolicy::Action::Stop: radioOff(); break;
        case WifiPolicy::Action::None: break;
    }
    bool on = online();
    if (on && !wasOnline) {
        log::printf("wifi: online %s as %s.local", WiFi.localIP().toString().c_str(), config::HOSTNAME);
        if (!mdnsStarted && MDNS.begin(config::HOSTNAME)) {
            MDNS.addService("http", "tcp", 80);
            mdnsStarted = true;
        }
    } else if (!on && wasOnline && policy.link() != WifiPolicy::Link::Off) {
        log::printf("wifi: link lost, reconnecting");
    }
    wasOnline = on;
}

bool online() { return policy.link() == WifiPolicy::Link::Online; }
bool configured() { return ssid.length() > 0; }
bool windowOpen() { return policy.windowOpen(millis()); }
WifiPolicy::Status status() { return policy.status(); }
bool devmode() { return policy.devmode(); }

void setDevmode(bool on) {
    if (on != policy.devmode()) log::printf("wifi: debug mode %s", on ? "on" : "off");
    policy.setDevmode(on);
}

void saveCredentials(const char* s, const char* p) {
    Preferences prefs;
    if (prefs.begin("wifi", false)) {
        prefs.putString("ssid", s);
        prefs.putString("pass", p);
        prefs.end();
    }
    ssid = s;
    pass = p;
    policy.setConfigured(true);
}

void forgetCredentials() {
    Preferences prefs;
    if (prefs.begin("wifi", false)) {
        prefs.clear();
        prefs.end();
    }
    ssid = pass = "";
    policy.setConfigured(false);
}

void appendDebug(String& j) {
    static const char* const STATUS[] = {"unconfigured", "connecting", "connected", "failed", "off"};
    char buf[240];
    snprintf(buf, sizeof(buf),
             "\"wifi_status\":\"%s\",\"wifi_window\":%s,\"devmode\":%s,\"ssid\":\"%s\",\"rssi\":%d,\"ip\":\"%s\"",
             STATUS[int(policy.status())], windowOpen() ? "true" : "false", devmode() ? "true" : "false",
             ssid.c_str(), online() ? int(WiFi.RSSI()) : 0,
             online() ? WiFi.localIP().toString().c_str() : "");
    j += buf;
}

}  // namespace owl::net
