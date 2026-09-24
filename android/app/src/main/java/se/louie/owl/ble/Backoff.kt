package se.louie.owl.ble

/** Reconnect delays: 1 s, 2 s, 4 s … capped at [maxMs]. Pure; unit-tested. */
class Backoff(private val baseMs: Long = 1000, private val maxMs: Long = 30_000) {
    private var attempt = 0

    fun next(): Long {
        val d = (baseMs shl minOf(attempt, 20)).coerceAtMost(maxMs)
        attempt++
        return d
    }

    fun reset() {
        attempt = 0
    }
}
