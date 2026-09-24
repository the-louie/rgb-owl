package se.louie.owl.wifi

import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class WifiFormTest {
    @Test
    fun saveNeedsPassingTest() {
        var f = WifiForm().edit(ssid = "net", password = "pw")
        assertFalse(f.canSave)
        f = f.startTest()
        assertFalse(f.canSave)
        assertFalse(f.canTest)
        f = f.finishTest(true, -50, null)
        assertTrue(f.canSave)
    }

    @Test
    fun failedTestBlocksSave() {
        val f = WifiForm().edit("net", "pw").startTest().finishTest(false, 0, "wrong password")
        assertFalse(f.canSave)
        assertTrue(f.canTest)
    }

    @Test
    fun editingAfterPassBlocksSave() {
        val f = WifiForm().edit("net", "pw").startTest().finishTest(true, -40, null)
        assertFalse(f.edit(password = "pw2").canSave)
        assertFalse(f.edit(ssid = "other").canSave)
        assertFalse(f.edit(password = "pw2").resultIsCurrent)
        assertTrue(f.edit(password = "pw2").edit(password = "pw").canSave) // back to the tested values
    }

    @Test
    fun blankSsidCannotBeTested() = assertFalse(WifiForm().edit(ssid = " ").canTest)
}
