package se.louie.owl.ble

import org.junit.Assert.assertEquals
import org.junit.Test

class BackoffTest {
    @Test
    fun doublesUpToCap() {
        val b = Backoff(1000, 5000)
        assertEquals(listOf(1000L, 2000L, 4000L, 5000L, 5000L), List(5) { b.next() })
    }

    @Test
    fun resetStartsOver() {
        val b = Backoff(1000, 30_000)
        b.next(); b.next()
        b.reset()
        assertEquals(1000L, b.next())
    }

    @Test
    fun noOverflowAfterManyAttempts() {
        val b = Backoff(1000, 30_000)
        repeat(100) { b.next() }
        assertEquals(30_000L, b.next())
    }
}
