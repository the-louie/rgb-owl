package se.louie.owl.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.Checkbox
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Slider
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import se.louie.owl.OwlViewModel
import se.louie.owl.protocol.formatMinutes

/** Brightness, speed, cycle timing, breathing colour, auto-cycle effects, night schedule, update project. */
@Composable
fun SettingsScreen(vm: OwlViewModel) {
    val state by vm.client.state.collectAsState()
    val config by vm.config.collectAsState()
    val project by vm.project.collectAsState()
    val effects by vm.client.effects.collectAsState()
    val s = state ?: return
    LaunchedEffect(Unit) { vm.refreshConfig() }

    Column(
        Modifier.verticalScroll(rememberScrollState()).padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(8.dp),
    ) {
        Section("Look")
        SettingSlider("Brightness", s.brightness, 1..255, { "${it * 100 / 255} %" }) { vm.set("brightness" to it) }
        SettingSlider("Speed", s.speed, 0..255, { "%.2f×".format(it / 128f) }) { vm.set("speed" to it) }
        SettingSlider("Time per effect", s.interval, 5..600, { if (it >= 60) "%.1f min".format(it / 60f) else "$it s" }) {
            vm.set("interval" to it)
        }
        SettingSlider("Crossfade", s.fade, 0..10_000, { "%.1f s".format(it / 1000f) }) { vm.set("fade" to it) }
        SettingSlider("Breathing colour", s.hue, 0..255, { "$it" }) { vm.set("hue" to it) }
        HueBar()

        config?.let { c ->
            Section("Effects in auto-cycle")
            effects.forEachIndexed { i, name ->
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Checkbox(checked = c.inCycle(i), onCheckedChange = { vm.setConfig("cycle" to c.withCycle(i, it)) })
                    Text(name.replaceFirstChar { it.uppercase() })
                }
            }

            Section("Night")
            Row(verticalAlignment = Alignment.CenterVertically) {
                Text("LEDs off at night", Modifier.weight(1f))
                Switch(checked = c.night, onCheckedChange = { vm.setConfig("night" to if (it) 1 else 0) })
            }
            if (c.night) {
                SettingSlider("From", c.nightFrom / 15, 0..95, { formatMinutes(it * 15) }) { vm.setConfig("night_from" to it * 15) }
                SettingSlider("To", c.nightTo / 15, 0..95, { formatMinutes(it * 15) }) { vm.setConfig("night_to" to it * 15) }
                if (!c.timeSet) Text("The owl's clock is not set yet.", color = MaterialTheme.colorScheme.error)
            }
        }

        Section("WiFi")
        WifiSection(vm)

        Section("Firmware updates")
        var url by remember(project) { mutableStateOf(project ?: "") }
        OutlinedTextField(
            value = url,
            onValueChange = { url = it },
            label = { Text("GitHub project (owner/repo or URL)") },
            singleLine = true,
            modifier = Modifier.fillMaxWidth(),
        )
        Button(onClick = { vm.setProject(url.trim()) }, enabled = url.trim() != (project ?: "")) { Text("Save project") }
        val update by vm.update.collectAsState()
        Text("Installed: ${s.version}")
        update.latest?.let { Text("Latest release: $it") }
        if (update.phase == "downloading" || update.phase == "verifying") {
            androidx.compose.material3.LinearProgressIndicator(
                progress = { update.percent / 100f },
                modifier = Modifier.fillMaxWidth(),
            )
        }
        update.message?.let { Text(it) }
        Button(onClick = vm::checkUpdate, enabled = !update.checking && !project.isNullOrEmpty()) {
            Text(if (update.checking) "Working…" else "Check for update")
        }
        Text(
            "A newer release is installed automatically. The owl also checks at every start and once a day.",
            style = MaterialTheme.typography.bodySmall,
        )
    }
}

@Composable
private fun Section(title: String) {
    HorizontalDivider(Modifier.padding(top = 8.dp))
    Text(title, style = MaterialTheme.typography.titleMedium, modifier = Modifier.padding(top = 8.dp))
}

/** Slider that follows the owl's value but only sends when the finger lifts. */
@Composable
private fun SettingSlider(label: String, value: Int, range: IntRange, format: (Int) -> String, onDone: (Int) -> Unit) {
    var local by remember(value) { mutableFloatStateOf(value.toFloat()) }
    Column {
        Row {
            Text(label, Modifier.weight(1f))
            Text(format(local.toInt()))
        }
        Slider(
            value = local,
            onValueChange = { local = it },
            onValueChangeFinished = { onDone(local.toInt()) },
            valueRange = range.first.toFloat()..range.last.toFloat(),
        )
    }
}

@Composable
private fun HueBar() {
    // FastLED hues 0..255 span the rainbow; pastel preview (low saturation) like the owl
    val colors = (0..6).map { Color.hsv(it * 60f % 360f, 0.45f, 1f) }
    Box(
        Modifier.fillMaxWidth().height(10.dp).clip(RoundedCornerShape(5.dp)).background(Brush.horizontalGradient(colors)),
    )
}
