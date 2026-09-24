package se.louie.owl.update

import android.app.DownloadManager
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Environment
import androidx.core.content.FileProvider
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.withContext
import se.louie.owl.BuildConfig
import java.io.File
import java.net.HttpURLConnection
import java.net.URL

/** Checks the owl project's GitHub releases for a newer app and installs it (Android asks the user). */
class AppUpdater(private val context: Context) {
    suspend fun check(project: String, includePre: Boolean): AppRelease? = withContext(Dispatchers.IO) {
        val c = URL("https://api.github.com/repos/$project/releases?per_page=10").openConnection() as HttpURLConnection
        c.setRequestProperty("Accept", "application/vnd.github+json")
        c.setRequestProperty("User-Agent", "owl-app")
        c.connectTimeout = 15_000
        c.readTimeout = 15_000
        try {
            if (c.responseCode != 200) error("GitHub request failed (${c.responseCode})")
            AppUpdate.pick(c.inputStream.bufferedReader().readText(), BuildConfig.VERSION_CODE, includePre)
        } finally {
            c.disconnect()
        }
    }

    /** Downloads the APK (progress 0..100 via [onProgress]) and opens the system installer. */
    suspend fun install(r: AppRelease, onProgress: (Int) -> Unit) {
        val dm = context.getSystemService(DownloadManager::class.java)
        val file = File(context.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS), "owl-app-${r.tag}.apk")
        file.delete()
        val id = dm.enqueue(
            DownloadManager.Request(Uri.parse(r.apkUrl))
                .setTitle("Owl app ${r.tag}")
                .setDestinationUri(Uri.fromFile(file)),
        )
        while (true) {
            dm.query(DownloadManager.Query().setFilterById(id)).use { cur ->
                if (!cur.moveToFirst()) error("download vanished")
                val status = cur.getInt(cur.getColumnIndexOrThrow(DownloadManager.COLUMN_STATUS))
                val done = cur.getLong(cur.getColumnIndexOrThrow(DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR))
                val total = cur.getLong(cur.getColumnIndexOrThrow(DownloadManager.COLUMN_TOTAL_SIZE_BYTES))
                if (total > 0) onProgress((done * 100 / total).toInt())
                when (status) {
                    DownloadManager.STATUS_SUCCESSFUL -> {
                        val uri = FileProvider.getUriForFile(context, "${context.packageName}.files", file)
                        context.startActivity(
                            Intent(Intent.ACTION_VIEW)
                                .setDataAndType(uri, "application/vnd.android.package-archive")
                                .addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_ACTIVITY_NEW_TASK),
                        )
                        return
                    }
                    DownloadManager.STATUS_FAILED -> error("download failed")
                }
            }
            delay(500)
        }
    }
}
