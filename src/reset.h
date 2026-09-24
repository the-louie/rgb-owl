#pragma once
// Recovery: holding the BOOT button (GPIO0) for 5 s forgets all bonded phones
// and the stored WiFi credentials, then restarts.

namespace owl::reset {

void begin();
void loop();

}  // namespace owl::reset
