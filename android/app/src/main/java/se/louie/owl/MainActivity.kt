package se.louie.owl

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.viewModels
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.core.content.ContextCompat
import se.louie.owl.ble.Connection
import se.louie.owl.ui.ConnectScreen

class MainActivity : ComponentActivity() {
    private val vm: OwlViewModel by viewModels()
    private val permissions = arrayOf(Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT)
    private val granted = mutableStateOf(false)

    private val request = registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) { result ->
        granted.value = result.values.all { it }
        if (granted.value) vm.start()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        granted.value = permissions.all { ContextCompat.checkSelfPermission(this, it) == PackageManager.PERMISSION_GRANTED }
        if (granted.value) vm.start() else request.launch(permissions)

        setContent {
            MaterialTheme(colorScheme = darkColorScheme()) {
                Surface(Modifier.fillMaxSize()) {
                    val connection by vm.client.connection.collectAsState()
                    when {
                        !granted.value -> Column(Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
                            Text("The app needs Bluetooth permission to find and control the owl.")
                            Button(onClick = { request.launch(permissions) }) { Text("Grant") }
                        }
                        !vm.client.bluetoothEnabled -> Text("Turn on Bluetooth.", Modifier.padding(16.dp))
                        connection is Connection.Ready -> ConnectedPlaceholder(vm)
                        else -> ConnectScreen(vm)
                    }
                }
            }
        }
    }
}

/** Replaced by the Main screen (T-26). */
@androidx.compose.runtime.Composable
private fun ConnectedPlaceholder(vm: OwlViewModel) {
    val state by vm.client.state.collectAsState()
    val effects by vm.client.effects.collectAsState()
    Column(Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
        Text("Connected", style = MaterialTheme.typography.headlineSmall)
        Text("Firmware ${state?.version}; showing ${effects.getOrNull(state?.current ?: -1)}")
        Button(onClick = { vm.forget() }) { Text("Forget this owl") }
    }
}
