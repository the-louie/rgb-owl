package se.louie.owl.protocol

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Test

class ProtocolTest {
    @Test
    fun verbOnly() = assertEquals("get", Protocol.command("get"))

    @Test
    fun encodesArgs() = assertEquals(
        "wifi_test ssid=My%20Net%21&pass=a%26b%3Dc%2B%25",
        Protocol.command("wifi_test", "ssid" to "My Net!", "pass" to "a&b=c+%"),
    )

    @Test
    fun encodesUtf8() = assertEquals("%C3%A5%C3%A4%C3%B6", Protocol.encode("åäö"))

    @Test
    fun numbersAndBools() = assertEquals("set brightness=80&on=false", Protocol.command("set", "brightness" to 80, "on" to false))

    @Test(expected = IllegalArgumentException::class)
    fun rejectsBadVerb() {
        Protocol.command("Set")
    }

    @Test
    fun parsesFirmwareState() {
        // exactly what src/ble.cpp sends
        val s = Protocol.parseState(
            "{\"on\":false,\"effect\":3,\"current\":5,\"auto\":true,\"interval\":60,\"fade\":2000," +
                "\"brightness\":30,\"speed\":101,\"hue\":160,\"version\":\"0.0.0-dev+abc\"}",
        )
        assertFalse(s.on)
        assertEquals(3, s.effect)
        assertEquals(5, s.current)
        assertEquals(30, s.brightness)
        assertEquals("0.0.0-dev+abc", s.version)
    }

    @Test
    fun ignoresUnknownStateFields() = assertEquals(7, Protocol.parseState("{\"effect\":7,\"future\":1}").effect)

    @Test
    fun parsesEffects() = assertEquals(listOf("plasma", "rain"), Protocol.parseEffects("[\"plasma\",\"rain\"]"))

    @Test
    fun parsesEvents() {
        val ok = Protocol.parseEvent("{\"type\":\"ok\",\"verb\":\"set\"}")
        assertEquals("ok", ok.type)
        assertEquals("set", ok.verb)
        assertNull(ok.msg)
        val err = Protocol.parseEvent("{\"type\":\"error\",\"verb\":\"set\",\"msg\":\"bad value\"}")
        assertEquals("bad value", err.msg)
    }
}
