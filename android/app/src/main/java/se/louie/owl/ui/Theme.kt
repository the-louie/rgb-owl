package se.louie.owl.ui

import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.size
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.ColorFilter
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import se.louie.owl.R

/** Bodforss brand colours and shades of them. */
object Bodforss {
    val Blue = Color(0xFF040D81)
    val BlueLight = Color(0xFFDDE0F7)     // tint of the blue (containers, the shown effect)
    val BlueLighter = Color(0xFFF1F2FB)   // cards
    val BlueGrey = Color(0xFF4A4F7A)      // secondary text
    val Green = Color(0xFF28A745)
    val GreenLight = Color(0xFFD4EDDA)
    val GreenDark = Color(0xFF155724)
    val Black = Color(0xFF000000)
    val White = Color(0xFFFFFFFF)
}

private val scheme = lightColorScheme(
    primary = Bodforss.Blue,
    onPrimary = Bodforss.White,
    primaryContainer = Bodforss.BlueLight,
    onPrimaryContainer = Bodforss.Blue,
    secondary = Bodforss.Green,
    onSecondary = Bodforss.White,
    secondaryContainer = Bodforss.GreenLight,
    onSecondaryContainer = Bodforss.GreenDark,
    tertiary = Bodforss.Green,
    onTertiary = Bodforss.White,
    background = Bodforss.White,
    onBackground = Bodforss.Black,
    surface = Bodforss.White,
    onSurface = Bodforss.Black,
    surfaceVariant = Bodforss.BlueLighter,
    onSurfaceVariant = Bodforss.BlueGrey,
    surfaceContainerLowest = Bodforss.White,
    surfaceContainerLow = Bodforss.BlueLighter,
    surfaceContainer = Bodforss.BlueLighter,
    surfaceContainerHigh = Bodforss.BlueLighter,
    surfaceContainerHighest = Bodforss.BlueLighter,
    outline = Color(0xFF9CA1CC),
    outlineVariant = Bodforss.BlueLight,
)

@Composable
fun OwlTheme(content: @Composable () -> Unit) = MaterialTheme(colorScheme = scheme, content = content)

/** The Bodforss owl mark (white PNG), tinted. */
@Composable
fun OwlMark(size: Dp = 32.dp, tint: Color = MaterialTheme.colorScheme.primary, modifier: Modifier = Modifier) =
    Image(
        painterResource(R.drawable.owl_mark),
        contentDescription = "Owl",
        colorFilter = ColorFilter.tint(tint),
        modifier = modifier.size(size),
    )
