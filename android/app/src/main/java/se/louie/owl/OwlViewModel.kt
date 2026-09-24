package se.louie.owl

import android.app.Application
import android.bluetooth.le.ScanResult
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import se.louie.owl.ble.OwlBleClient
import kotlinx.serialization.json.contentOrNull
import kotlinx.serialization.json.jsonPrimitive
import kotlinx.serialization.json.booleanOrNull
import se.louie.owl.protocol.OwlConfig
import se.louie.owl.wifi.WifiForm
import se.louie.owl.wifi.WifiNetwork
import se.louie.owl.protocol.Protocol

data class WifiInfo(val configured: Boolean, val ssid: String, val status: String, val ip: String)

data class FoundOwl(val address: String, val name: String?, val rssi: Int)

class OwlViewModel(app: Application) : AndroidViewModel(app) {
    val client = OwlBleClient(app, viewModelScope)

    private val _found = MutableStateFlow<List<FoundOwl>>(emptyList())
    val found: StateFlow<List<FoundOwl>> = _found.asStateFlow()
    private val _scanning = MutableStateFlow(false)
    val scanning: StateFlow<Boolean> = _scanning.asStateFlow()
    private val _error = MutableStateFlow<String?>(null)
    val error: StateFlow<String?> = _error.asStateFlow()
    private var scanJob: Job? = null

    private val _config = MutableStateFlow<OwlConfig?>(null)
    val config: StateFlow<OwlConfig?> = _config.asStateFlow()
    private val _project = MutableStateFlow<String?>(null)
    val project: StateFlow<String?> = _project.asStateFlow()

    init {
        viewModelScope.launch {
            client.events.collect { e ->
                when (e.type) {
                    "error" -> {
                        _error.value = "${e.verb}: ${e.msg}"
                        if (e.verb == "wifi_test") _wifiForm.value = _wifiForm.value.finishTest(false, 0, e.msg)
                        if (e.verb == "wifi_scan") _wifiScanning.value = false
                    }
                    "config" -> _config.value = Protocol.parseConfig(e.fields)
                    "project" -> _project.value = e.fields["project"]?.jsonPrimitive?.contentOrNull
                    "wifi_net" -> {
                        val n = WifiNetwork(e.str("ssid") ?: "", e.int("rssi") ?: 0, e.fields["secure"]?.jsonPrimitive?.booleanOrNull ?: true)
                        _networks.value = (_networks.value.filter { it.ssid != n.ssid } + n).sortedByDescending { it.rssi }
                    }
                    "wifi_scan_done" -> _wifiScanning.value = false
                    "wifi_test" -> _wifiForm.value = _wifiForm.value.finishTest(
                        e.fields["ok"]?.jsonPrimitive?.booleanOrNull == true, e.int("rssi") ?: 0, e.msg,
                    )
                    "debug" -> _debug.value = _debug.value + e.fields.filterKeys { it != "type" }
                        .mapValues { it.value.jsonPrimitive.content }
                    "wifi_info" -> _wifiInfo.value = WifiInfo(
                        e.fields["configured"]?.jsonPrimitive?.booleanOrNull == true, e.str("ssid") ?: "",
                        e.str("status") ?: "", e.str("ip") ?: "",
                    )
                }
            }
        }
    }

    /** Asks the owl for the settings that are not in the state notification. */
    fun refreshConfig() {
        send("config")
        send("project")
    }

    /** Sets config-event settings (cycle, night…) and re-reads them. */
    fun setConfig(vararg args: Pair<String, Any>) {
        set(*args)
        send("config")
    }

    private val _networks = MutableStateFlow<List<WifiNetwork>>(emptyList())
    val networks: StateFlow<List<WifiNetwork>> = _networks.asStateFlow()
    private val _wifiScanning = MutableStateFlow(false)
    val wifiScanning: StateFlow<Boolean> = _wifiScanning.asStateFlow()
    private val _wifiForm = MutableStateFlow(WifiForm())
    val wifiForm: StateFlow<WifiForm> = _wifiForm.asStateFlow()
    private val _wifiInfo = MutableStateFlow<WifiInfo?>(null)
    val wifiInfo: StateFlow<WifiInfo?> = _wifiInfo.asStateFlow()

    fun wifiRefresh() = send("wifi_info")

    fun wifiScan() {
        _networks.value = emptyList()
        _wifiScanning.value = true
        send("wifi_scan")
    }

    fun wifiEdit(ssid: String = _wifiForm.value.ssid, password: String = _wifiForm.value.password) {
        _wifiForm.value = _wifiForm.value.edit(ssid, password)
    }

    fun wifiTest() {
        val f = _wifiForm.value.startTest()
        _wifiForm.value = f
        send(Protocol.command("wifi_test", "ssid" to f.ssid, "pass" to f.password))
    }

    /** Saves the tested credentials; the dialog closes when [wifiInfo] reports them. */
    fun wifiSave() {
        val f = _wifiForm.value
        if (!f.canSave) return
        send(Protocol.command("wifi_save", "ssid" to f.ssid, "pass" to f.password))
        send("wifi_info")
    }

    fun wifiForget() {
        send("wifi_forget")
        send("wifi_info")
    }

    fun wifiReset() {
        _wifiForm.value = WifiForm(ssid = _wifiInfo.value?.ssid ?: "")
    }

    private val _debug = MutableStateFlow<Map<String, String>>(emptyMap())
    val debug: StateFlow<Map<String, String>> = _debug.asStateFlow()

    fun refreshDebug() {
        send("debug")
        send("wifi_info")
    }

    fun setDevmode(on: Boolean) = send(Protocol.command("devmode", "on" to if (on) 1 else 0))

    fun test(mode: String, index: Int = 0, r: Int = 0, g: Int = 0, b: Int = 0) =
        send(Protocol.command("test", "mode" to mode, "index" to index, "r" to r, "g" to g, "b" to b))

    fun setProject(url: String) = send(Protocol.command("project", "url" to url))

    /** Called once permissions are granted: reconnect to the remembered owl, if any. */
    fun start() {
        client.rememberedAddress?.let { if (client.connection.value is se.louie.owl.ble.Connection.Idle) client.connect(it) }
    }

    fun scan() {
        scanJob?.cancel()
        _found.value = emptyList()
        _scanning.value = true
        scanJob = viewModelScope.launch {
            try {
                kotlinx.coroutines.withTimeoutOrNull(15_000) {
                    client.scan().collect { r: ScanResult ->
                        val owl = FoundOwl(r.device.address, r.scanRecord?.deviceName, r.rssi)
                        _found.value = (_found.value.filter { it.address != owl.address } + owl).sortedByDescending { it.rssi }
                    }
                }
            } catch (e: Exception) {
                _error.value = e.message
            } finally {
                _scanning.value = false
            }
        }
    }

    fun connect(address: String) {
        scanJob?.cancel()
        client.connect(address)
    }

    fun forget() = client.disconnect(forget = true)

    /** Sends `set k=v&…`; errors come back as events. */
    fun set(vararg args: Pair<String, Any>) = send(Protocol.command("set", *args))

    fun send(line: String) {
        viewModelScope.launch {
            try {
                client.send(line)
            } catch (e: Exception) {
                _error.value = e.message
            }
        }
    }

    fun clearError() {
        _error.value = null
    }
}
