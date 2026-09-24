package se.louie.owl

import org.junit.Assert.assertTrue
import org.junit.Test

class SmokeTest {
    @Test
    fun versionNameIsSet() = assertTrue(BuildConfig.VERSION_NAME.isNotEmpty())
}
