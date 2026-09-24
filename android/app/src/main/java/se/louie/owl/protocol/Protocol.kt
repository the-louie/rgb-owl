package se.louie.owl.protocol

import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.contentOrNull
import kotlinx.serialization.json.intOrNull
import kotlinx.serialization.json.jsonPrimitive

/** Owl state as sent on the `state` characteristic (firmware: src/ble.cpp stateJson). */
@Serializable
data class OwlState(
    val on: Boolean = true,
    val effect: Int = 0,
    val current: Int = 0,
    val next: Int = -1, // crossfade target, -1 when not fading
    @SerialName("auto") val autoCycle: Boolean = true,
    val interval: Int = 60,
    val fade: Int = 2000,
    val brightness: Int = 128,
    val speed: Int = 128,
    val hue: Int = 160,
    val wifi: String = "",
    val devmode: Boolean = false,
    val version: String = "",
)

/** `config` event: settings that do not fit the state notification (firmware: src/ble.cpp). */
@Serializable
data class OwlConfig(
    val cycle: Long = 0x7FFFFFFF,
    val night: Boolean = true,
    @SerialName("night_from") val nightFrom: Int = 23 * 60,
    @SerialName("night_to") val nightTo: Int = 7 * 60,
    @SerialName("time_set") val timeSet: Boolean = false,
    val tz: String = "",
) {
    fun inCycle(effect: Int) = effect in 0..30 && (cycle shr effect) and 1L == 1L

    /** Mask with [effect] switched on or off. */
    fun withCycle(effect: Int, on: Boolean): Long = if (on) cycle or (1L shl effect) else cycle and (1L shl effect).inv()
}

/** "HH:MM" for minutes after midnight. */
fun formatMinutes(m: Int): String = "%02d:%02d".format((m / 60) % 24, m % 60)

/** Reply on the `event` characteristic. `fields` keeps the whole object for event types added later. */
data class OwlEvent(val type: String, val verb: String?, val msg: String?, val fields: JsonObject) {
    fun str(key: String): String? = fields[key]?.jsonPrimitive?.contentOrNull
    fun int(key: String): Int? = fields[key]?.jsonPrimitive?.intOrNull
}

object Protocol {
    private val json = Json { ignoreUnknownKeys = true }

    /** Builds `verb k=v&k=v` with keys/values percent-encoded (firmware: lib/owl/protocol.h Command). */
    fun command(verb: String, vararg args: Pair<String, Any>): String {
        require(verb.isNotEmpty() && verb.all { it in 'a'..'z' || it in '0'..'9' || it == '_' }) { "bad verb: $verb" }
        if (args.isEmpty()) return verb
        return verb + " " + args.joinToString("&") { (k, v) -> encode(k) + "=" + encode(v.toString()) }
    }

    /** Percent-encodes everything except RFC 3986 unreserved characters (UTF-8). */
    fun encode(s: String): String = buildString {
        for (b in s.toByteArray(Charsets.UTF_8)) {
            val c = b.toInt() and 0xff
            val ch = c.toChar()
            if (ch in 'A'..'Z' || ch in 'a'..'z' || ch in '0'..'9' || ch in "-._~") append(ch)
            else append('%').append("0123456789ABCDEF"[c shr 4]).append("0123456789ABCDEF"[c and 15])
        }
    }

    fun parseState(text: String): OwlState = json.decodeFromString(OwlState.serializer(), text)

    fun parseEffects(text: String): List<String> = json.decodeFromString(text)

    fun parseConfig(fields: JsonObject): OwlConfig = json.decodeFromJsonElement(OwlConfig.serializer(), fields)

    fun parseEvent(text: String): OwlEvent {
        val o = json.decodeFromString(JsonObject.serializer(), text)
        fun str(k: String) = o[k]?.jsonPrimitive?.contentOrNull
        return OwlEvent(str("type") ?: "", str("verb"), str("msg"), o)
    }
}
