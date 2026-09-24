package se.louie.owl.time

import java.time.Instant
import java.time.ZoneId
import java.time.ZoneOffset
import java.time.zone.ZoneOffsetTransitionRule
import java.time.zone.ZoneOffsetTransitionRule.TimeDefinition

/**
 * Converts a java.time zone (e.g. Europe/Stockholm) to a POSIX TZ string for the owl's newlib,
 * e.g. "<+01>-1<+02>,M3.5.0/2,M10.5.0/3". Zones without a recurring DST rule become a fixed offset.
 */
object PosixTz {
    fun of(zone: ZoneId, now: Instant = Instant.now()): String {
        val rules = zone.rules
        val std = rules.getStandardOffset(now)
        val tr = rules.transitionRules
        if (tr.size != 2) return name(std) + offset(std)
        // the rule that moves away from standard time starts DST
        val (start, end) = if (tr[0].offsetBefore == tr[0].standardOffset) tr[0] to tr[1] else tr[1] to tr[0]
        val dst = start.offsetAfter
        val s = rule(start) ?: return name(std) + offset(std)
        val e = rule(end) ?: return name(std) + offset(std)
        return name(std) + offset(std) + name(dst) + (if (dst.totalSeconds - std.totalSeconds != 3600) offset(dst) else "") +
            ",$s,$e"
    }

    /** "<+0530>" style name (POSIX needs a name; numeric ones are quoted). */
    private fun name(o: ZoneOffset): String {
        val t = o.totalSeconds
        val sign = if (t < 0) "-" else "+"
        val a = Math.abs(t)
        val h = a / 3600
        val m = a / 60 % 60
        return "<$sign%02d${if (m != 0) "%02d".format(m) else ""}>".format(h)
    }

    /** POSIX offset: hours WEST of UTC, so the sign is inverted. */
    private fun offset(o: ZoneOffset): String {
        val t = -o.totalSeconds
        val sign = if (t < 0) "-" else ""
        val a = Math.abs(t)
        val h = a / 3600
        val m = a / 60 % 60
        val s = a % 60
        return sign + h + (if (m != 0 || s != 0) ":%02d".format(m) else "") + (if (s != 0) ":%02d".format(s) else "")
    }

    /** "Mm.w.d/time" in local wall time before the transition; null if not expressible. */
    private fun rule(r: ZoneOffsetTransitionRule): String? {
        val dow = r.dayOfWeek ?: return null
        val dom = r.dayOfMonthIndicator
        val week = when {
            dom < 0 -> 5  // last <weekday> of the month (dom = -1)
            dom + 6 >= r.month.maxLength() -> 5  // TZDB writes "lastSun" as "Sun on or after 25th" (31-day month)
            (dom - 1) % 7 == 0 -> (dom - 1) / 7 + 1  // "<weekday> on or after 1/8/15/22"
            else -> return null
        }
        val d = dow.value % 7  // POSIX: 0 = Sunday
        var secs = if (r.isMidnightEndOfDay) 86400 else r.localTime.toSecondOfDay()
        secs += when (r.timeDefinition) {
            TimeDefinition.UTC -> r.offsetBefore.totalSeconds
            TimeDefinition.STANDARD -> r.offsetBefore.totalSeconds - r.standardOffset.totalSeconds
            else -> 0
        }
        val h = secs / 3600
        val m = secs / 60 % 60
        return "M${r.month.value}.$week.$d/$h" + if (m != 0) ":%02d".format(m) else ""
    }
}
