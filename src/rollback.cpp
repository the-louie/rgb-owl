#include "rollback.h"

#include <Arduino.h>
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
    const esp_partition_t* bad = esp_ota_get_last_invalid_partition();
    rolled = bad != nullptr;
    log::printf("ota: running %s (%s)%s%s", running->label, pending ? "pending verify" : "valid",
                rolled ? ", rolled back from " : "", rolled ? bad->label : "");
}

void loop() {
    if (!pending || !shouldMarkValid(ble::advertising(), millis(), config::ROLLBACK_GRACE_MS)) return;
    if (esp_ota_mark_app_valid_cancel_rollback() == ESP_OK) {
        pending = false;
        log::printf("ota: image marked valid");
    }
}

const char* state() { return pending ? "pending" : "valid"; }
bool rolledBack() { return rolled; }

}  // namespace owl::rollback
