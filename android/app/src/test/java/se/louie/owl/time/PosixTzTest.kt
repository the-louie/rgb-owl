package se.louie.owl.time

import org.junit.Assert.assertEquals
import org.junit.Test
import java.time.Instant
import java.time.ZoneId

class PosixTzTest {
    private val now = Instant.parse("2026-09-24T10:00:00Z")
    private fun tz(id: String) = PosixTz.of(ZoneId.of(id), now)

    @Test
    fun stockholm() = assertEquals("<+01>-1<+02>,M3.5.0/2,M10.5.0/3", tz("Europe/Stockholm"))

    @Test
    fun london() = assertEquals("<+00>0<+01>,M3.5.0/1,M10.5.0/2", tz("Europe/London"))

    @Test
    fun newYork() = assertEquals("<-05>5<-04>,M3.2.0/2,M11.1.0/2", tz("America/New_York"))

    @Test
    fun sydneySouthernHemisphere() = assertEquals("<+10>-10<+11>,M10.1.0/2,M4.1.0/3", tz("Australia/Sydney"))

    @Test
    fun tokyoNoDst() = assertEquals("<+09>-9", tz("Asia/Tokyo"))

    @Test
    fun kolkataHalfHour() = assertEquals("<+0530>-5:30", tz("Asia/Kolkata"))

    @Test
    fun utc() = assertEquals("<+00>0", tz("UTC"))
}
