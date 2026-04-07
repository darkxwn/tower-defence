plugins {
    id("com.android.application")
}

android {
    namespace = "com.darkxwn.billvlad.towerdefence"
    compileSdk = 35
    ndkVersion = "23.2.8568313" // Пример для r23c. Поставь ту, что у тебя в папке.

    defaultConfig {
        applicationId = "com.darkxwn.billvlad.towerdefence"
        minSdk = 26
        targetSdk = 35
        versionCode = 2
        versionName = "0.1"

        externalNativeBuild {
            cmake {
                arguments("-DANDROID_STL=c++_shared")
                cppFlags("-std=c++17")
                abiFilters("arm64-v8a", "armeabi-v7a")
            }
        }
    }

    externalNativeBuild {
        cmake {
            // Используем file(), чтобы Gradle сам вычислил абсолютный путь
            path = file("../../CMakeLists.txt")
        }
    }

    sourceSets {
        getByName("main") {
            manifest.srcFile("src/main/AndroidManifest.xml")
            // Исправленный синтаксис (без ворнингов)
            assets.setSrcDirs(listOf("../../assets", "../../data"))
        }
    }
    packaging {
        jniLibs {
            // Если Gradle находит две одинаковые либы, он просто берет первую
            pickFirsts.add("lib/arm64-v8a/libtower-defence.so")
            pickFirsts.add("lib/armeabi-v7a/libtower-defence.so")
            pickFirsts.add("*libsfml-*.so")
        }
    }
}

dependencies {
    // Минимальные зависимости для NativeActivity
    implementation("androidx.appcompat:appcompat:1.6.1")
    implementation("androidx.startup:startup-runtime:1.1.1")
    implementation("com.google.android.material:material:1.9.0")
}