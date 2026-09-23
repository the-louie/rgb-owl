#pragma once
// Firmware updates over WiFi: ArduinoOTA (pio upload --upload-port owl.local)
// and POST /update (.bin upload from the web UI). No authentication (see TODO.md).

namespace owl::ota {

void begin();  // registers /update; call before the first http::loop()
void loop();

}  // namespace owl::ota
