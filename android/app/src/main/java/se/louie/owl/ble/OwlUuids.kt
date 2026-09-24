package se.louie.owl.ble

import java.util.UUID

/** Must match src/ble.h in the firmware. */
object OwlUuids {
    val SERVICE: UUID = UUID.fromString("4f574c00-8a1b-4c2e-9d3f-2b1a6c7e0001")
    val STATE: UUID = UUID.fromString("4f574c00-8a1b-4c2e-9d3f-2b1a6c7e0002")
    val COMMAND: UUID = UUID.fromString("4f574c00-8a1b-4c2e-9d3f-2b1a6c7e0003")
    val EVENT: UUID = UUID.fromString("4f574c00-8a1b-4c2e-9d3f-2b1a6c7e0004")
    val EFFECTS: UUID = UUID.fromString("4f574c00-8a1b-4c2e-9d3f-2b1a6c7e0005")
    val CCCD: UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")
}
