package se.louie.owl.wifi

/** Result of the owl's `wifi_test`. */
sealed interface TestResult {
    data object None : TestResult
    data object Running : TestResult
    data class Passed(val rssi: Int) : TestResult
    data class Failed(val reason: String) : TestResult
}

/**
 * WiFi dialog state. Mirrors the owl's rule (lib/owl/wifi_test.h): Save is only allowed for
 * exactly the credentials that passed the last test. Pure; unit-tested.
 */
data class WifiForm(
    val ssid: String = "",
    val password: String = "",
    val result: TestResult = TestResult.None,
    private val testedSsid: String? = null,
    private val testedPassword: String? = null,
) {
    fun edit(ssid: String = this.ssid, password: String = this.password) = copy(ssid = ssid, password = password)

    fun startTest() = copy(result = TestResult.Running, testedSsid = ssid, testedPassword = password)

    fun finishTest(passed: Boolean, rssi: Int, reason: String?) =
        copy(result = if (passed) TestResult.Passed(rssi) else TestResult.Failed(reason ?: "failed"))

    val canTest: Boolean get() = ssid.isNotBlank() && result != TestResult.Running

    val canSave: Boolean
        get() = result is TestResult.Passed && ssid == testedSsid && password == testedPassword

    /** The result shown only applies while the fields are unchanged since the test. */
    val resultIsCurrent: Boolean get() = ssid == testedSsid && password == testedPassword
}

data class WifiNetwork(val ssid: String, val rssi: Int, val secure: Boolean)
