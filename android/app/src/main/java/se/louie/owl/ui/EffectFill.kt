package se.louie.owl.ui

/**
 * Which part of an effect button is filled with the primary colour, as fractions of its width
 * (start, end), or null for none. The shown effect is full; during a crossfade (progress 0..1)
 * the incoming effect fills from the left (0..p) while the outgoing one shrinks towards the
 * right (p..1), so both together always cover exactly one button. Pure; unit-tested.
 */
fun effectFill(effect: Int, current: Int, next: Int, progress: Float): Pair<Float, Float>? {
    val p = progress.coerceIn(0f, 1f)
    return when {
        next < 0 -> if (effect == current) 0f to 1f else null
        effect == next -> if (p > 0f) 0f to p else null
        effect == current -> if (p < 1f) p to 1f else null
        else -> null
    }
}
