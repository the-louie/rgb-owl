package se.louie.owl.update

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Test

class AppUpdateTest {
    @Test
    fun versionCodes() {
        assertEquals(1_020_399, AppUpdate.versionCode("v1.2.3"))
        assertEquals(10_099, AppUpdate.versionCode("0.1.0"))
        assertNull(AppUpdate.versionCode("v1.2.3-rc.1"))
        assertNull(AppUpdate.versionCode("nightly"))
    }

    private val json = """[
        {"tag_name":"v1.3.0","prerelease":true,"draft":false,"assets":[{"name":"owl-app.apk","browser_download_url":"https://x/1.3.0.apk"}]},
        {"tag_name":"v1.2.0","prerelease":false,"draft":false,"assets":[{"name":"owl-app.apk","browser_download_url":"https://x/1.2.0.apk"},{"name":"owl-firmware.bin","browser_download_url":"https://x/fw"}]},
        {"tag_name":"v1.1.0","prerelease":false,"draft":false,"assets":[]},
        {"tag_name":"v9.0.0","prerelease":false,"draft":true,"assets":[{"name":"owl-app.apk","browser_download_url":"https://x/9.apk"}]}
    ]"""

    @Test
    fun picksNewestStable() = assertEquals("https://x/1.2.0.apk", AppUpdate.pick(json, 1, includePre = false)?.apkUrl)

    @Test
    fun preReleaseChannel() = assertEquals("v1.3.0", AppUpdate.pick(json, 1, includePre = true)?.tag)

    @Test
    fun nothingNewer() = assertNull(AppUpdate.pick(json, AppUpdate.versionCode("v1.2.0")!!, includePre = false))

    @Test
    fun devBuildOfSameBaseIsNotReplacedByOlder() =
        // app built as 1.2.1-dev.3 (code 1_020_103) must not "update" to v1.2.0
        assertNull(AppUpdate.pick(json, 1_020_103, includePre = false))
}
