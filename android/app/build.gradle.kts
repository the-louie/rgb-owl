import java.io.ByteArrayOutputStream
import java.util.Properties

plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
    id("org.jetbrains.kotlin.plugin.compose")
    id("org.jetbrains.kotlin.plugin.serialization")
}

// Same scheme as the firmware (tools/version.py): vX.Y.Z tag -> "X.Y.Z",
// N commits later -> "X.Y.(Z+1)-dev.N". versionCode must grow for self-update:
// X*1_000_000 + Y*10_000 + Z*100 + (99 for a release, min(N, 98) for a dev build).
fun gitDescribe(): String? = try {
    val out = ByteArrayOutputStream()
    exec {
        commandLine("git", "describe", "--tags", "--match", "v[0-9]*", "--long")
        standardOutput = out
        isIgnoreExitValue = true
        errorOutput = ByteArrayOutputStream()
    }
    out.toString().trim().ifEmpty { null }
} catch (e: Exception) {
    null
}

val (owlVersionName, owlVersionCode) = run {
    val d = gitDescribe() ?: return@run "0.0.0-dev" to 1
    val (tag, n) = d.substringBeforeLast('-').let { it.substringBeforeLast('-') to it.substringAfterLast('-').toInt() }
    val (ma, mi, pa) = tag.removePrefix("v").substringBefore('-').split('.').map { it.toInt() }
    if (n == 0) "$ma.$mi.$pa" to (ma * 1_000_000 + mi * 10_000 + pa * 100 + 99)
    else "$ma.$mi.${pa + 1}-dev.$n" to (ma * 1_000_000 + mi * 10_000 + (pa + 1) * 100 + minOf(n, 98))
}

// Release signing: keys/owl-app.properties locally (gitignored), or OWL_KEYSTORE_* env vars in CI.
// The same key must sign every release, or the app's self-update cannot install over the old one.
val signing = Properties().apply {
    val f = rootProject.file("../keys/owl-app.properties")
    if (f.exists()) f.inputStream().use { load(it) }
    System.getenv("OWL_KEYSTORE_FILE")?.let { setProperty("storeFile", it) }
    System.getenv("OWL_KEYSTORE_PASSWORD")?.let { setProperty("storePassword", it); setProperty("keyPassword", it) }
    if (getProperty("keyAlias") == null) setProperty("keyAlias", "owl")
}

android {
    namespace = "se.louie.owl"
    compileSdk = 35
    defaultConfig {
        applicationId = "se.louie.owl"
        minSdk = 31 // Android 12: BLUETOOTH_SCAN/CONNECT permissions, no location needed
        targetSdk = 35
        versionCode = owlVersionCode
        versionName = owlVersionName
    }
    signingConfigs {
        if (signing.getProperty("storeFile") != null) {
            create("release") {
                storeFile = file(signing.getProperty("storeFile"))
                storePassword = signing.getProperty("storePassword")
                keyAlias = signing.getProperty("keyAlias")
                keyPassword = signing.getProperty("keyPassword")
            }
        }
    }
    buildTypes {
        release {
            isMinifyEnabled = false
            signingConfig = signingConfigs.findByName("release")
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions { jvmTarget = "17" }
    buildFeatures {
        compose = true
        buildConfig = true
    }
}

dependencies {
    val bom = platform("androidx.compose:compose-bom:2024.12.01")
    implementation(bom)
    implementation("androidx.activity:activity-compose:1.9.3")
    implementation("androidx.compose.material3:material3")
    implementation("androidx.lifecycle:lifecycle-viewmodel-compose:2.8.7")
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-android:1.9.0")
    implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.7.3")
    testImplementation("junit:junit:4.13.2")
    testImplementation("org.jetbrains.kotlinx:kotlinx-coroutines-test:1.9.0")
}
