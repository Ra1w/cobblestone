add_library(core STATIC
    src/types/id.cpp
    src/types/tag.cpp
    src/types/timestamp.cpp
    src/types/title.cpp
    src/entities/entity.cpp
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
