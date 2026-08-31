package com.oguzhanp.chess.engine

// NativeBridge artik ayri bir dosyada: NativeBridge.kt

actual fun engineVersion(): String = NativeBridge.version()

actual fun engineSelfTest(): Int = NativeBridge.selftest()
