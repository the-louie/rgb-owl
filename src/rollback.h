#pragma once
// OTA rollback guard: a new image stays "pending verify" until BLE is up and it has run
// for ROLLBACK_GRACE_MS; if it resets before that, the bootloader boots the previous image.

namespace owl::rollback {

void begin();  // logs the image state and any rollback that just happened
void loop();
const char* state();  // "valid", "pending" or "unknown"
bool rolledBack();    // this boot is the result of a rollback
// Call right before restarting into a freshly written image (installer, ArduinoOTA).
void expectNewImage();

}  // namespace owl::rollback
