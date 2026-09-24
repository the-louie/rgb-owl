package se.louie.owl.update

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import se.louie.owl.protocol.Protocol

class UpdateViewTest {
    private fun ev(json: String) = Protocol.parseEvent(json)

    @Test
    fun upToDate() {
        val v = UpdateView().startCheck()
            .on(ev("{\"type\":\"update_info\",\"current\":\"1.0.0\",\"latest\":\"v1.0.0\",\"newer\":false}"))
        assertFalse(v.checking)
        assertEquals("v1.0.0", v.latest)
        assertEquals("Up to date", v.message)
    }

    @Test
    fun newerInstallsWithProgress() {
        var v = UpdateView().startCheck()
            .on(ev("{\"type\":\"update_info\",\"current\":\"1.0.0\",\"latest\":\"v1.1.0\",\"newer\":true}"))
        assertTrue(v.checking)
        v = v.on(ev("{\"type\":\"update\",\"state\":\"downloading\",\"pct\":45}"))
        assertEquals(45, v.percent)
        v = v.on(ev("{\"type\":\"update\",\"state\":\"installed\",\"pct\":100}"))
        assertEquals("Installed, the owl restarts…", v.message)
    }

    @Test
    fun newerWithoutSignedFirmware() {
        val v = UpdateView().startCheck().on(
            ev("{\"type\":\"update_info\",\"latest\":\"v2.0.0\",\"newer\":true,\"msg\":\"release v2.0.0 has no signed firmware\"}"),
        )
        assertFalse(v.checking)
        assertEquals("release v2.0.0 has no signed firmware", v.message)
    }

    @Test
    fun failedInstall() {
        val v = UpdateView(checking = true).on(ev("{\"type\":\"update\",\"state\":\"failed\",\"msg\":\"bad signature\"}"))
        assertFalse(v.checking)
        assertEquals("Update failed: bad signature", v.message)
    }

    @Test
    fun checkRefused() {
        val v = UpdateView().startCheck().on(ev("{\"type\":\"error\",\"verb\":\"update_check\",\"msg\":\"wifi not configured\"}"))
        assertFalse(v.checking)
        assertEquals("wifi not configured", v.message)
    }
}
