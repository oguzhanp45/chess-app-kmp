package com.oguzhanp.chess

interface Platform {
    val name: String
}

expect fun getPlatform(): Platform