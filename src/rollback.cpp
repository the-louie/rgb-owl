#include "rollback.h"

#include <Arduino.h>
#include <Preferences.h>
#include <esp_ota_ops.h>

#include "ble.h"
#include "config.h"
#include "log.h"
#include "owl/rollback.h"

// Arduino core hook: returning true stops initArduino() from marking every image valid at once.
extern "C" bool verifyRollbackLater() { return true; }

namespace owl::rollback {

static bool pending = false;
static bool rolled = false;

void begin() {
    esp_ota_img_states_t st;
    const esp_partition_t* running = esp_ota_get_running_partition();
    pending = esp_ota_get_state_partition(running, &st) == ESP_OK && st == ESP_OTA_IMG_PENDING_VERIFY;
    // otadata keeps old invalid entries around, so esp_ota_get_last_invalid_partition() is not a
    // reliable "this boot rolled back" signal. Instead expectNewImage() records the slot we should
    // come up in; landing anywhere else means the bootloader rolled back.
    Preferences p;
    char expected[17] = "";
    if (p.begin("ota", false)) {
        p.getString("expect", expected, sizeof(expected));
        rolled = expected[0] && strcmp(expected, running->label) != 0;
        // keep the expectation while the new image is still on probation (it may yet crash);
        // it is cleared once reported as a rollback or once the image is marked valid
        if (rolled || (expected[0] && !pending)) p.remove("expect");
        p.end();
    }
    log::printf("ota: running %s (%s)%s%s", running->label, pending ? "pending verify" : "valid",
                rolled ? ", ROLLED BACK from " : "", rolled ? expected : "");
}

void loop() {
    if (!pending || !shouldMarkValid(ble::advertising(), millis(), config::ROLLBACK_GRACE_MS)) return;
    if (esp_ota_mark_app_valid_cancel_rollback() == ESP_OK) {
        pending = false;
        Preferences p;
        if (p.begin("ota", false)) {
            p.remove("expect");
            p.end();
        }
        log::printf("ota: image marked valid");
    }
}

const char* state() { return pending ? "pending" : "valid"; }

void expectNewImage() {
    const esp_partition_t* next = esp_ota_get_boot_partition();  // Update.end() already switched it
    Preferences p;
    if (next && p.begin("ota", false)) {
        p.putString("expect", next->label);
        p.end();
    }
}
bool rolledBack() { return rolled; }

}  // namespace owl::rollback
