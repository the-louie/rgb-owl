package se.louie.owl.ble

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothProfile
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.os.Build
import android.os.ParcelUuid
import android.util.Log
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.callbackFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withTimeout
import se.louie.owl.protocol.OwlEvent
import se.louie.owl.protocol.OwlState
import se.louie.owl.protocol.Protocol

private const val TAG = "OwlBle"

sealed interface Connection {
    data object Idle : Connection
    data class Connecting(val address: String) : Connection
    data class Ready(val address: String, val name: String?) : Connection
    data class Failed(val reason: String) : Connection
}

/**
 * GATT client for one owl. Android allows one outstanding GATT operation at a time, so every
 * operation goes through [op], which holds a mutex until the matching callback completes it.
 * Callers must hold BLUETOOTH_SCAN / BLUETOOTH_CONNECT (requested by the UI).
 */
@SuppressLint("MissingPermission")
class OwlBleClient(private val context: Context, private val scope: CoroutineScope) {
    private val adapter = context.getSystemService(BluetoothManager::class.java).adapter
    private val prefs = context.getSharedPreferences("owl", Context.MODE_PRIVATE)

    private val _connection = MutableStateFlow<Connection>(Connection.Idle)
    val connection: StateFlow<Connection> = _connection.asStateFlow()
    private val _state = MutableStateFlow<OwlState?>(null)
    val state: StateFlow<OwlState?> = _state.asStateFlow()
    private val _effects = MutableStateFlow<List<String>>(emptyList())
    val effects: StateFlow<List<String>> = _effects.asStateFlow()
    private val _events = MutableSharedFlow<OwlEvent>(extraBufferCapacity = 16)
    val events: SharedFlow<OwlEvent> = _events.asSharedFlow()

    private var gatt: BluetoothGatt? = null
    private val opLock = Mutex()
    private var pending: CompletableDeferred<Any?>? = null
    private var wanted: String? = null // address we should stay connected to
    private var reconnectJob: Job? = null
    private val backoff = Backoff()
    private var ready = false   // current link finished setUp()
    private var refusals = 0    // unpaired links dropped before setup

    companion object {
        /** Shown when the owl drops an unpaired phone: its pairing window (3 min after power-on) is closed. */
        const val REFUSED = "The owl refused to pair. New phones can only pair during the first 3 minutes " +
            "after the owl is powered on: unplug it for 2 s, plug it back in, then connect again."
    }

    /** Address of the owl this phone is bonded with, if any. */
    val rememberedAddress: String? get() = prefs.getString("address", null)

    val bluetoothEnabled: Boolean get() = adapter?.isEnabled == true

    /** Owls advertising the service UUID, deduplicated by the caller. */
    fun scan(): Flow<ScanResult> = callbackFlow {
        val scanner = adapter.bluetoothLeScanner ?: run { close(); return@callbackFlow }
        val cb = object : ScanCallback() {
            override fun onScanResult(callbackType: Int, result: ScanResult) {
                trySend(result)
            }

            override fun onScanFailed(errorCode: Int) {
                close(IllegalStateException("scan failed: $errorCode"))
            }
        }
        val filter = ScanFilter.Builder().setServiceUuid(ParcelUuid(OwlUuids.SERVICE)).build()
        val settings = ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build()
        scanner.startScan(listOf(filter), settings, cb)
        awaitClose { scanner.stopScan(cb) }
    }

    /** Connects and keeps reconnecting until [disconnect]. The address is remembered. */
    fun connect(address: String) {
        refusals = 0
        wanted = address
        prefs.edit().putString("address", address).apply()
        backoff.reset()
        open(address)
    }

    fun disconnect(forget: Boolean = false) {
        wanted = null
        reconnectJob?.cancel()
        gatt?.close()
        gatt = null
        failPending("disconnected")
        if (forget) prefs.edit().remove("address").apply()
        _connection.value = Connection.Idle
    }

    /** Sends a command line; the reply arrives on [events]. */
    suspend fun send(line: String) {
        val g = gatt ?: error("not connected")
        val chr = g.getService(OwlUuids.SERVICE)?.getCharacteristic(OwlUuids.COMMAND) ?: error("no command characteristic")
        op {
            val bytes = line.toByteArray(Charsets.UTF_8)
            if (Build.VERSION.SDK_INT >= 33) {
                g.writeCharacteristic(chr, bytes, BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT)
            } else {
                @Suppress("DEPRECATION")
                chr.value = bytes
                @Suppress("DEPRECATION")
                g.writeCharacteristic(chr)
            }
        }
    }

    private fun open(address: String) {
        gatt?.close()
        val device = adapter.getRemoteDevice(address)
        _connection.value = Connection.Connecting(address)
        gatt = device.connectGatt(context, false, callback, BluetoothDevice.TRANSPORT_LE)
    }

    private fun scheduleReconnect() {
        val address = wanted ?: return
        reconnectJob?.cancel()
        reconnectJob = scope.launch {
            delay(backoff.next())
            if (wanted == address) open(address)
        }
    }

    /** Runs one GATT operation: [start] must return true if Android accepted it. */
    private suspend fun op(timeoutMs: Long = 10_000, start: () -> Any?): Any? = opLock.withLock {
        val d = CompletableDeferred<Any?>()
        pending = d
        val accepted = start()
        if (accepted == false || (accepted is Int && accepted != BluetoothGatt.GATT_SUCCESS)) {
            pending = null
            error("GATT operation rejected")
        }
        try {
            withTimeout(timeoutMs) { d.await() }
        } finally {
            pending = null
        }
    }

    private fun complete(value: Any?) {
        pending?.complete(value)
    }

    private fun failPending(reason: String) {
        pending?.completeExceptionally(IllegalStateException(reason))
    }

    /** After connect: MTU, services, bonding (triggered by the first encrypted read), notifications, initial reads. */
    private suspend fun setUp(g: BluetoothGatt) {
        op { g.requestMtu(247) }
        op { g.discoverServices() }
        val svc = g.getService(OwlUuids.SERVICE) ?: error("not an owl (service missing)")
        for (c in svc.characteristics) Log.d(TAG, "char ${c.uuid} handle ${c.instanceId} props ${c.properties}")
        // First encrypted read: Android shows the system PIN dialog if this phone is not bonded yet,
        // so allow time for typing the PIN.
        val effects = read(g, svc.getCharacteristic(OwlUuids.EFFECTS), timeoutMs = 90_000)
        _effects.value = Protocol.parseEffects(effects)
        enableNotify(g, svc.getCharacteristic(OwlUuids.STATE))
        enableNotify(g, svc.getCharacteristic(OwlUuids.EVENT))
        _state.value = Protocol.parseState(read(g, svc.getCharacteristic(OwlUuids.STATE)))
    }

    private suspend fun read(g: BluetoothGatt, chr: BluetoothGattCharacteristic, timeoutMs: Long = 10_000): String {
        val v = op(timeoutMs) { g.readCharacteristic(chr) } as ByteArray
        Log.d(TAG, "read ${chr.uuid} (handle ${chr.instanceId}): ${v.size} B ${v.joinToString("") { "%02x".format(it) }.take(64)}")
        return v.toString(Charsets.UTF_8)
    }

    private suspend fun enableNotify(g: BluetoothGatt, chr: BluetoothGattCharacteristic) {
        g.setCharacteristicNotification(chr, true)
        val cccd = chr.getDescriptor(OwlUuids.CCCD) ?: error("no CCCD")
        val v = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
        op {
            if (Build.VERSION.SDK_INT >= 33) {
                g.writeDescriptor(cccd, v)
            } else {
                @Suppress("DEPRECATION")
                cccd.value = v
                @Suppress("DEPRECATION")
                g.writeDescriptor(cccd)
            }
        }
    }

    private fun onValue(uuid: java.util.UUID, value: ByteArray) {
        val text = value.toString(Charsets.UTF_8)
        try {
            when (uuid) {
                OwlUuids.STATE -> _state.value = Protocol.parseState(text)
                OwlUuids.EVENT -> _events.tryEmit(Protocol.parseEvent(text))
            }
        } catch (e: Exception) {
            Log.w(TAG, "bad notification on $uuid: $text", e)
        }
    }

    private val callback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(g: BluetoothGatt, status: Int, newState: Int) {
            if (g != gatt) return
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                scope.launch {
                    try {
                        setUp(g)
                        ready = true
                        refusals = 0
                        backoff.reset()
                        _connection.value = Connection.Ready(g.device.address, g.device.name)
                    } catch (e: Exception) {
                        Log.w(TAG, "setup failed", e)
                        _connection.value = Connection.Failed(e.message ?: "setup failed")
                        g.disconnect()
                    }
                }
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                failPending("disconnected (status $status)")
                g.close()
                if (gatt == g) gatt = null
                val wasReady = ready
                ready = false
                // An unpaired phone that is dropped before setup = the owl's pairing window is closed.
                // Retrying cannot help, so stop and say what to do.
                if (!wasReady && g.device.bondState != BluetoothDevice.BOND_BONDED && ++refusals >= 2) {
                    wanted = null
                    prefs.edit().remove("address").apply()
                    _connection.value = Connection.Failed(REFUSED)
                    return
                }
                if (wanted != null) {
                    _connection.value = Connection.Connecting(g.device.address)
                    scheduleReconnect()
                }
            }
        }

        override fun onMtuChanged(g: BluetoothGatt, mtu: Int, status: Int) = complete(mtu)

        override fun onServicesDiscovered(g: BluetoothGatt, status: Int) {
            if (status == BluetoothGatt.GATT_SUCCESS) complete(null) else failPending("discovery failed: $status")
        }

        override fun onCharacteristicRead(g: BluetoothGatt, chr: BluetoothGattCharacteristic, value: ByteArray, status: Int) {
            Log.d(TAG, "onCharacteristicRead ${chr.uuid} status=$status ${value.size} B")
            if (status == BluetoothGatt.GATT_SUCCESS) complete(value) else failPending("read failed: $status")
        }

        @Deprecated("API < 33")
        override fun onCharacteristicRead(g: BluetoothGatt, chr: BluetoothGattCharacteristic, status: Int) {
            @Suppress("DEPRECATION")
            onCharacteristicRead(g, chr, chr.value ?: ByteArray(0), status)
        }

        override fun onCharacteristicWrite(g: BluetoothGatt, chr: BluetoothGattCharacteristic, status: Int) {
            if (status == BluetoothGatt.GATT_SUCCESS) complete(null) else failPending("write failed: $status")
        }

        override fun onDescriptorWrite(g: BluetoothGatt, d: BluetoothGattDescriptor, status: Int) {
            if (status == BluetoothGatt.GATT_SUCCESS) complete(null) else failPending("descriptor write failed: $status")
        }

        override fun onCharacteristicChanged(g: BluetoothGatt, chr: BluetoothGattCharacteristic, value: ByteArray) =
            onValue(chr.uuid, value)

        @Deprecated("API < 33")
        override fun onCharacteristicChanged(g: BluetoothGatt, chr: BluetoothGattCharacteristic) {
            @Suppress("DEPRECATION")
            onValue(chr.uuid, chr.value ?: ByteArray(0))
        }
    }
}
