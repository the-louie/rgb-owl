#pragma once
// Firmware updates over WiFi: ArduinoOTA (pio upload --upload-port owl.local)
// and POST /update (.bin upload from the web UI). Both require OWL_OTA_PASSWORD
// ([owl] ota_password in platformio.ini); the rest of the web UI is open.

namespace owl::ota {

void begin();  // registers /update; call before the first http::loop()
void loop();

}  // namespace owl::ota
