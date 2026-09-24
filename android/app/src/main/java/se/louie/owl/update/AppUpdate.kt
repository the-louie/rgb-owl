package se.louie.owl.update

import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable
import kotlinx.serialization.json.Json

/** GitHub release JSON (only the fields we need). */
@Serializable
data class GhRelease(
    @SerialName("tag_name") val tag: String = "",
    val prerelease: Boolean = false,
    val draft: Boolean = false,
    val assets: List<GhAsset> = emptyList(),
)

@Serializable
data class GhAsset(val name: String = "", @SerialName("browser_download_url") val url: String = "")

data class AppRelease(val tag: String, val versionCode: Int, val apkUrl: String)

/** Finding a newer app build in the owl project's releases. Pure; unit-tested. */
object AppUpdate {
    const val APK_ASSET = "owl-app.apk"
    private val json = Json { ignoreUnknownKeys = true }

    /** versionCode of release tag vX.Y.Z, as android/app/build.gradle.kts computes it; null if not X.Y.Z. */
    fun versionCode(tag: String): Int? {
        val m = Regex("""^v?(\d+)\.(\d+)\.(\d+)$""").matchEntire(tag) ?: return null
        val (ma, mi, pa) = m.destructured
        return ma.toInt() * 1_000_000 + mi.toInt() * 10_000 + pa.toInt() * 100 + 99
    }

    /** Newest stable (or also pre-release) release with an APK whose versionCode beats [current]. */
    fun pick(releasesJson: String, current: Int, includePre: Boolean): AppRelease? =
        json.decodeFromString<List<GhRelease>>(releasesJson)
            .filter { !it.draft && (includePre || !it.prerelease) }
            .mapNotNull { r ->
                val code = versionCode(r.tag) ?: return@mapNotNull null
                val apk = r.assets.firstOrNull { it.name == APK_ASSET } ?: return@mapNotNull null
                AppRelease(r.tag, code, apk.url)
            }
            .filter { it.versionCode > current }
            .maxByOrNull { it.versionCode }
}
