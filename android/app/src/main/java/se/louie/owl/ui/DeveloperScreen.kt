package se.louie.owl.ui

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ExperimentalLayoutApi
import androidx.compose.foundation.layout.FlowRow
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Slider
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.delay
import se.louie.owl.OwlViewModel

/** Hidden screen (tap the firmware version 5×): debug mode, status fields, test patterns. */
@OptIn(ExperimentalLayoutApi::class)
@Composable
fun DeveloperScreen(vm: OwlViewModel) {
    val state by vm.client.state.collectAsState()
    val debug by vm.debug.collectAsState()
    val info by vm.wifiInfo.collectAsState()
    var index by remember { mutableFloatStateOf(0f) }
    LaunchedEffect(Unit) {
        while (true) {  // refresh while the screen is open
            vm.refreshDebug()
            delay(3000)
        }
    }

    Column(Modifier.verticalScroll(rememberScrollState()).padding(16.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
        Text("Developer", style = MaterialTheme.typography.headlineSmall)
        Row(verticalAlignment = Alignment.CenterVertically) {
            Column(Modifier.weight(1f)) {
                Text("Debug mode")
                Text(
                    "WiFi stays on with web UI, debug API and ArduinoOTA until off or reboot.",
                    style = MaterialTheme.typography.bodySmall,
                )
            }
            Switch(checked = state?.devmode == true, onCheckedChange = vm::setDevmode)
        }
        info?.let { if (it.ip.isNotEmpty()) Text("http://${it.ip}/  (${it.status})", fontFamily = FontFamily.Monospace) }

        Text("Status", style = MaterialTheme.typography.titleMedium)
        debug.toSortedMap().forEach { (k, v) -> Text("$k: $v", fontFamily = FontFamily.Monospace, style = MaterialTheme.typography.bodySmall) }

        Text("Test patterns", style = MaterialTheme.typography.titleMedium)
        FlowRow(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            Button(onClick = { vm.test("walk") }) { Text("Walk") }
            Button(onClick = { vm.test("solid", r = 255, g = 255, b = 255) }) { Text("White") }
            Button(onClick = { vm.test("solid", r = 255) }) { Text("Red") }
            Button(onClick = { vm.test("solid", g = 255) }) { Text("Green") }
            Button(onClick = { vm.test("solid", b = 255) }) { Text("Blue") }
            Button(onClick = { vm.test("off") }) { Text("Off") }
        }
        Text("Index ${index.toInt()} (pixel = strip LED, column 0 = rightmost, row 0 = bottom)")
        Slider(value = index, onValueChange = { index = it }, valueRange = 0f..61f, steps = 60)
        FlowRow(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            OutlinedButton(onClick = { vm.test("pixel", index.toInt()) }) { Text("Pixel") }
            OutlinedButton(onClick = { vm.test("column", index.toInt()) }) { Text("Column") }
            OutlinedButton(onClick = { vm.test("row", index.toInt()) }) { Text("Row") }
            Button(onClick = { vm.test("none") }) { Text("Back to effects") }
        }
    }
}
