package se.louie.owl.ui

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Test

class EffectFillTest {
    @Test
    fun steadyStateOnlyCurrentIsFull() {
        assertEquals(0f to 1f, effectFill(2, current = 2, next = -1, progress = 0f))
        assertNull(effectFill(3, current = 2, next = -1, progress = 0f))
    }

    @Test
    fun midFadeSplitsAtProgress() {
        assertEquals(0f to 0.3f, effectFill(5, current = 2, next = 5, progress = 0.3f)) // incoming grows from the left
        assertEquals(0.3f to 1f, effectFill(2, current = 2, next = 5, progress = 0.3f)) // outgoing shrinks to the right
        assertNull(effectFill(4, current = 2, next = 5, progress = 0.3f))
    }

    @Test
    fun fadeEnds() {
        assertEquals(0f to 1f, effectFill(5, current = 2, next = 5, progress = 1f))
        assertNull(effectFill(2, current = 2, next = 5, progress = 1f))
        assertNull(effectFill(5, current = 2, next = 5, progress = 0f))
        assertEquals(0f to 1f, effectFill(2, current = 2, next = 5, progress = 0f))
    }
}
