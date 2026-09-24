package se.louie.owl.ui

import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.clickable
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.itemsIndexed
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Snackbar
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import se.louie.owl.OwlViewModel

/** On/off, effect grid (outlined = selected, filled = currently shown), auto-cycle. */
@Composable
fun MainScreen(vm: OwlViewModel, onSecret: () -> Unit = {}) {
    var taps by remember { mutableIntStateOf(0) }
    val state by vm.client.state.collectAsState()
    val effects by vm.client.effects.collectAsState()
    val error by vm.error.collectAsState()
    val s = state ?: return

    Column(Modifier.fillMaxSize().padding(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
        Row(Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically) {
            Text("🦉 Owl", style = MaterialTheme.typography.headlineMedium, modifier = Modifier.weight(1f))
            Text(if (s.on) "On" else "Off", modifier = Modifier.padding(end = 8.dp))
            Switch(checked = s.on, onCheckedChange = { vm.set("on" to if (it) 1 else 0) })
        }
        Row(Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically) {
            Text("Auto-cycle", modifier = Modifier.weight(1f))
            Switch(checked = s.autoCycle, onCheckedChange = { vm.set("auto" to if (it) 1 else 0) })
        }
        LazyVerticalGrid(
            columns = GridCells.Adaptive(140.dp),
            horizontalArrangement = Arrangement.spacedBy(8.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp),
            modifier = Modifier.weight(1f),
        ) {
            itemsIndexed(effects) { i, name ->
                val shown = i == s.current
                Card(
                    onClick = { vm.set("effect" to i) },
                    colors = if (shown) CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.primaryContainer)
                    else CardDefaults.cardColors(),
                    border = if (i == s.effect) BorderStroke(2.dp, MaterialTheme.colorScheme.primary) else null,
                ) {
                    Text(
                        name.replaceFirstChar { it.uppercase() },
                        Modifier.fillMaxWidth().padding(vertical = 20.dp),
                        textAlign = TextAlign.Center,
                    )
                }
            }
        }
        Text(
            "Firmware ${s.version}",
            style = MaterialTheme.typography.bodySmall,
            modifier = Modifier.clickable { if (++taps >= 5) { taps = 0; onSecret() } },
        )
        error?.let {
            Snackbar(action = { TextButton(onClick = vm::clearError) { Text("OK") } }) { Text(it) }
        }
        Spacer(Modifier.height(4.dp))
    }
}
