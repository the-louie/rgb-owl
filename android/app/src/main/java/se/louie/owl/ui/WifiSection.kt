package se.louie.owl.ui

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.text.input.VisualTransformation
import androidx.compose.ui.unit.dp
import se.louie.owl.OwlViewModel
import se.louie.owl.wifi.TestResult

/** Current WiFi + "Change WiFi" dialog: owl scan list or manual SSID, password, Test, Save (after a pass). */
@Composable
fun WifiSection(vm: OwlViewModel) {
    val info by vm.wifiInfo.collectAsState()
    var editing by remember { mutableStateOf(false) }
    LaunchedEffect(Unit) { vm.wifiRefresh() }

    val i = info
    Text(
        when {
            i == null -> "…"
            !i.configured -> "Not configured. The owl uses WiFi only for updates and debugging."
            else -> "Network: ${i.ssid} (${i.status}${if (i.ip.isNotEmpty()) ", ${i.ip}" else ""})"
        },
    )
    Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
        Button(onClick = {
            vm.wifiReset()
            vm.wifiScan()
            editing = true
        }) { Text(if (i?.configured == true) "Change WiFi" else "Set up WiFi") }
        if (i?.configured == true) OutlinedButton(onClick = vm::wifiForget) { Text("Forget") }
    }
    if (editing) WifiDialog(vm, onClose = { editing = false })
}

@Composable
private fun WifiDialog(vm: OwlViewModel, onClose: () -> Unit) {
    val form by vm.wifiForm.collectAsState()
    val networks by vm.networks.collectAsState()
    val scanning by vm.wifiScanning.collectAsState()
    val info by vm.wifiInfo.collectAsState()
    var showPw by remember { mutableStateOf(false) }
    var saving by remember { mutableStateOf(false) }

    // close once the owl reports the saved network
    LaunchedEffect(info, saving) { if (saving && info?.configured == true && info?.ssid == form.ssid) onClose() }

    AlertDialog(
        onDismissRequest = onClose,
        title = { Text("WiFi for updates") },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                Row {
                    Text(if (scanning) "Scanning…" else "Networks the owl can see", Modifier.weight(1f),
                        style = MaterialTheme.typography.labelMedium)
                    if (!scanning) Text("Rescan", Modifier.clickable { vm.wifiScan() }, color = MaterialTheme.colorScheme.primary)
                }
                LazyColumn(Modifier.heightIn(max = 160.dp)) {
                    items(networks, key = { it.ssid }) { n ->
                        Text(
                            "${n.ssid}  ${n.rssi} dBm${if (n.secure) " 🔒" else ""}",
                            Modifier.fillMaxWidth().clickable { vm.wifiEdit(ssid = n.ssid) }.padding(vertical = 6.dp),
                            color = if (n.ssid == form.ssid) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.onSurface,
                        )
                    }
                }
                OutlinedTextField(form.ssid, { vm.wifiEdit(ssid = it) }, label = { Text("Network (SSID)") }, singleLine = true)
                OutlinedTextField(
                    form.password, { vm.wifiEdit(password = it) },
                    label = { Text("Password") }, singleLine = true,
                    visualTransformation = if (showPw) VisualTransformation.None else PasswordVisualTransformation(),
                    trailingIcon = { TextButton(onClick = { showPw = !showPw }) { Text(if (showPw) "Hide" else "Show") } },
                )
                if (form.resultIsCurrent) when (val r = form.result) {
                    TestResult.Running -> Text("Testing… (up to 15 s)")
                    is TestResult.Passed -> Text("✓ The owl connected (${r.rssi} dBm)", color = MaterialTheme.colorScheme.primary)
                    is TestResult.Failed -> Text("✗ ${r.reason}", color = MaterialTheme.colorScheme.error)
                    TestResult.None -> {}
                }
                if (!form.canSave) Text("Test the network before saving.", style = MaterialTheme.typography.bodySmall)
            }
        },
        confirmButton = {
            Row {
                TextButton(onClick = vm::wifiTest, enabled = form.canTest) { Text("Test") }
                Button(onClick = {
                    saving = true
                    vm.wifiSave()
                }, enabled = form.canSave) { Text("Save") }
            }
        },
        dismissButton = { TextButton(onClick = onClose) { Text("Cancel") } },
    )
}
