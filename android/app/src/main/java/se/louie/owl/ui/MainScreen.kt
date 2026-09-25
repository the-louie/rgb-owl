package se.louie.owl.ui

import androidx.compose.animation.core.Animatable
import androidx.compose.animation.core.LinearEasing
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.ui.draw.clip
import androidx.compose.ui.geometry.Rect
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Outline
import androidx.compose.ui.graphics.Shape
import androidx.compose.ui.unit.Density
import androidx.compose.ui.unit.LayoutDirection
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
    val fadeProgress = remember { Animatable(0f) }
    LaunchedEffect(s.next) {
        fadeProgress.snapTo(0f)
        if (s.next >= 0) fadeProgress.animateTo(1f, tween(durationMillis = s.fade.coerceIn(0, 10_000), easing = LinearEasing))
    }

    Column(Modifier.fillMaxSize().padding(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
        Row(Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically) {
            OwlMark(36.dp, modifier = Modifier.padding(end = 10.dp))
            Text("Owl", style = MaterialTheme.typography.headlineMedium, color = MaterialTheme.colorScheme.primary,
                modifier = Modifier.weight(1f))
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
                // Each button doubles as a progress bar for the owl's crossfade (see effectFill).
                val fill = effectFill(i, s.current, s.next, fadeProgress.value)
                val label = name.replaceFirstChar { it.uppercase() }
                Card(
                    onClick = { vm.set("effect" to i) },
                    colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerHighest),
                    border = if (i == s.effect) BorderStroke(2.dp, MaterialTheme.colorScheme.primary) else null,
                ) {
                    Box(Modifier.fillMaxWidth()) {
                        EffectLabel(label, MaterialTheme.colorScheme.onSurface)
                        if (fill != null) {
                            // primary-coloured part, with the label in white clipped to the same span
                            Box(
                                Modifier.matchParentSize()
                                    .clip(SpanShape(fill.first, fill.second))
                                    .background(MaterialTheme.colorScheme.primary),
                            ) { EffectLabel(label, MaterialTheme.colorScheme.onPrimary) }
                        }
                    }
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

@Composable
private fun EffectLabel(text: String, color: androidx.compose.ui.graphics.Color) = Text(
    text,
    Modifier.fillMaxWidth().padding(vertical = 20.dp),
    textAlign = TextAlign.Center,
    color = color,
)

/** Rectangle covering [start, end] of the width (fractions). */
private class SpanShape(private val start: Float, private val end: Float) : Shape {
    override fun createOutline(size: Size, layoutDirection: LayoutDirection, density: Density) =
        Outline.Rectangle(Rect(size.width * start, 0f, size.width * end, size.height))
}
