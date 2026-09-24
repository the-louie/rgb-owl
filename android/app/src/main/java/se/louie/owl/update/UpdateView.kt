package se.louie.owl.update

import se.louie.owl.protocol.OwlEvent

/** What the Settings screen shows about firmware updates, folded from owl events. Pure; unit-tested. */
data class UpdateView(
    val checking: Boolean = false,
    val latest: String? = null,
    val newer: Boolean = false,
    val message: String? = null,
    val phase: String? = null, // downloading | verifying | installed | failed
    val percent: Int = 0,
) {
    fun startCheck() = copy(checking = true, message = null, phase = null, percent = 0)

    fun on(e: OwlEvent): UpdateView = when {
        e.type == "update_info" -> copy(
            checking = e.fields["newer"]?.toString() == "true" && e.msg == null, // stays busy while installing
            latest = e.str("latest")?.ifEmpty { null },
            newer = e.fields["newer"]?.toString() == "true",
            message = e.msg ?: if (e.fields["newer"]?.toString() == "true") "Installing…" else "Up to date",
        )
        e.type == "update" -> {
            val phase = e.str("state")
            copy(
                checking = phase == "downloading" || phase == "verifying" || phase == "installed",
                phase = phase,
                percent = e.int("pct") ?: percent,
                message = when (phase) {
                    "failed" -> "Update failed: ${e.msg}"
                    "installed" -> "Installed, the owl restarts…"
                    else -> message
                },
            )
        }
        e.type == "error" && e.verb == "update_check" -> copy(checking = false, message = e.msg)
        else -> this
    }
}
