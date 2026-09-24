package se.louie.owl.ui

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.safeDrawingPadding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import se.louie.owl.OwlViewModel
import se.louie.owl.ble.Connection

/** Shown until an owl is connected: scan, pick, pair (system PIN dialog). */
@Composable
fun ConnectScreen(vm: OwlViewModel) {
    val connection by vm.client.connection.collectAsState()
    val found by vm.found.collectAsState()
    val scanning by vm.scanning.collectAsState()

    Column(Modifier.fillMaxSize().safeDrawingPadding().padding(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
        Text("🦉 Owl", style = MaterialTheme.typography.headlineMedium)
        when (val c = connection) {
            is Connection.Connecting -> Row(verticalAlignment = Alignment.CenterVertically) {
                CircularProgressIndicator(Modifier.padding(end = 12.dp))
                Text("Connecting to ${c.address}…\nFirst time: enter the owl's 6-digit PIN when Android asks. Pair from this app, not from Android's Bluetooth settings.")
            }
            is Connection.Failed -> Text("Connection failed: ${c.reason}", color = MaterialTheme.colorScheme.error)
            else -> Text("Find your owl nearby. It must be powered on.")
        }
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            Button(onClick = vm::scan, enabled = !scanning) { Text(if (scanning) "Scanning…" else "Scan") }
            if (connection !is Connection.Idle) Button(onClick = { vm.forget() }) { Text("Cancel") }
        }
        LazyColumn(verticalArrangement = Arrangement.spacedBy(8.dp)) {
            items(found, key = { it.address }) { owl ->
                Card(Modifier.fillMaxWidth().clickable { vm.connect(owl.address) }) {
                    Column(Modifier.padding(12.dp)) {
                        Text(owl.name ?: "Owl", style = MaterialTheme.typography.titleMedium)
                        Text("${owl.address} · ${owl.rssi} dBm", style = MaterialTheme.typography.bodySmall)
                        when {
                            owl.bonded -> Text("Paired with this phone", color = MaterialTheme.colorScheme.primary)
                            owl.pairingOpen == true -> Text("Ready to pair (PIN needed)", color = MaterialTheme.colorScheme.primary)
                            owl.pairingOpen == false -> Text(
                                "Not accepting new phones now. Unplug the owl for 2 s, then pair within 3 minutes.",
                                color = MaterialTheme.colorScheme.error,
                            )
                        }
                    }
                }
            }
        }
    }
}
