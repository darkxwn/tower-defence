import java.util.Properties
import java.io.FileInputStream

// Функция для загрузки свойств
val localProperties = Properties().apply {
    val localPropertiesFile = rootProject.file("local.properties")
    if (localPropertiesFile.exists()) {
        load(FileInputStream(localPropertiesFile))
    }
}

// Хелпер для получения пропсов (сначала из командной строки -P, потом из local.properties)
fun getSignProperty(key: String, localKey: String): String {
    return project.findProperty(key)?.toString() 
        ?: localProperties.getProperty(localKey) 
        ?: ""
}

plugins {
    id("com.android.application")
}

android {
    namespace = "com.darkxwn.billvlad.towerdefence"
    compileSdk = 35
    ndkVersion = "27.2.12479018"
	
	signingConfigs {
        create("release") {
            // Файл ключа должен лежать в папке android/app/
            storeFile = file("release.keystore")
            
            // Пароли, которые ты вводил при создании через keytool
            storePassword = getSignProperty("releaseStorePassword", "release.keystore.password")
            keyAlias = getSignProperty("releaseKeyAlias", "release.key.alias")
            keyPassword = getSignProperty("releaseKeyPassword", "release.key.password")
            
            // Опционально для Android 16/HyperOS: принудительно используем V2/V3 подпись
            enableV1Signing = true
            enableV2Signing = true
            enableV3Signing = true
            enableV4Signing = true
        }
    }
	
	buildTypes {
        getByName("release") {
            isMinifyEnabled = false
            signingConfig = signingConfigs.getByName("release")
            
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
        
        getByName("debug") {
            // Отладочная сборка по умолчанию подписывается дебажным ключом Google
            isMinifyEnabled = false
        }
    }

    splits {
        abi {
            isEnable = true // Включает разделение
            reset() // Сбрасывает список архитектур по умолчанию
            include("arm64-v8a", "armeabi-v7a", "x86_64") // Список нужных архитектур
            isUniversalApk = true // Если true, создаст еще и один общий тяжелый APK
        }
    }

    defaultConfig {
        applicationId = "com.darkxwn.billvlad.towerdefence"
        minSdk = 26
        targetSdk = 36
        versionCode = 3
        versionName = "0.3a"

        externalNativeBuild {
			cmake {
				cppFlags("-std=c++20")
				arguments(
					"-Wno-dev",
					"-DANDROID_STL=c++_shared",
					"-DANDROID_CPP_FEATURES=rtti exceptions", // Добавь это
					"-DCMAKE_CXX_STANDARD=20",
					"-DCMAKE_CXX_STANDARD_REQUIRED=ON",
					"-DSFML_OS_ANDROID=ON"
				)
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
            assets.setSrcDirs(listOf("../../assets", "../../data"))
        }
    }
    packaging {
		jniLibs {
			// Обязательно разрешаем дубликаты для STL, так как SFML может пытаться тащить свой
			pickFirsts.add("**/libc++_shared.so")
			pickFirsts.add("**/libsfml-*.so")
			pickFirsts.add("**/libtower-defence.so")
		}
	}
}

dependencies {
    // Минимальные зависимости для NativeActivity
    implementation("androidx.appcompat:appcompat:1.6.1")
    implementation("androidx.startup:startup-runtime:1.1.1")
    implementation("com.google.android.material:material:1.9.0")
}