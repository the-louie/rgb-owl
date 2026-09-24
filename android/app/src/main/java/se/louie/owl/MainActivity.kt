package se.louie.owl

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.safeDrawingPadding
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.viewModels
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.core.content.ContextCompat
import se.louie.owl.ble.Connection
import se.louie.owl.ui.ConnectScreen
import se.louie.owl.ui.MainScreen

class MainActivity : ComponentActivity() {
    private val vm: OwlViewModel by viewModels()
    private val permissions = arrayOf(Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT)
    private val granted = mutableStateOf(false)

    private val request = registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) { result ->
        granted.value = result.values.all { it }
        if (granted.value) vm.start()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        enableEdgeToEdge()  // targetSdk 35 draws edge-to-edge anyway; screens pad for the system bars
        super.onCreate(savedInstanceState)
        granted.value = permissions.all { ContextCompat.checkSelfPermission(this, it) == PackageManager.PERMISSION_GRANTED }
        if (granted.value) vm.start() else request.launch(permissions)

        setContent {
            MaterialTheme(colorScheme = darkColorScheme()) {
                Surface(Modifier.fillMaxSize()) {
                    val connection by vm.client.connection.collectAsState()
                    when {
                        !granted.value -> Column(Modifier.safeDrawingPadding().padding(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
                            Text("The app needs Bluetooth permission to find and control the owl.")
                            Button(onClick = { request.launch(permissions) }) { Text("Grant") }
                        }
                        !vm.client.bluetoothEnabled -> Text("Turn on Bluetooth.", Modifier.safeDrawingPadding().padding(16.dp))
                        connection is Connection.Ready -> OwlTabs(vm)
                        else -> ConnectScreen(vm)
                    }
                }
            }
        }
    }
}

@androidx.compose.runtime.Composable
private fun OwlTabs(vm: OwlViewModel) {
    var tab by androidx.compose.runtime.saveable.rememberSaveable { mutableStateOf(0) }
    var dev by androidx.compose.runtime.saveable.rememberSaveable { mutableStateOf(false) }
    val tabs = if (dev) listOf("Owl", "Settings", "Developer") else listOf("Owl", "Settings")
    androidx.compose.material3.Scaffold(
        bottomBar = {
            androidx.compose.material3.NavigationBar {
                tabs.forEachIndexed { i, label ->
                    NavigationBarItem(
                        selected = tab == i,
                        onClick = { tab = i },
                        icon = { Text(listOf("🦉", "⚙", "🛠")[i]) },
                        label = { Text(label) },
                    )
                }
            }
        },
    ) { pad ->
        androidx.compose.foundation.layout.Box(Modifier.padding(pad)) {
            when (tab) {
                0 -> MainScreen(vm, onSecret = { dev = true; tab = 2 })
                1 -> se.louie.owl.ui.SettingsScreen(vm)
                else -> se.louie.owl.ui.DeveloperScreen(vm)
            }
        }
    }
}
