cmake_minimum_required(VERSION 3.10)
project(mon_projet C)

set(CMAKE_C_STANDARD 99)

add_executable(mon_projet main.c
        GAME_TYPES.c
        GAME_TYPES.h

)

IF(WIN32)
    target_link_libraries(mon_projet -lmingw32 -lallegro-5.0.10-monolith-md-debug)
ELSE()
    INCLUDE_DIRECTORIES(/usr/local/Cellar/allegro/5.2.8.0/include)
    LINK_DIRECTORIES(/usr/local/Cellar/allegro/5.2.8.0/lib)
    file(GLOB LIBRARIES "/usr/local/Cellar/allegro/5.2.8.0/lib/*.dylib")
    message("LIBRARIES = ${LIBRARIES}")
    TARGET_LINK_LIBRARIES(mon_projet ${LIBRARIES})
ENDIF(WIN32)