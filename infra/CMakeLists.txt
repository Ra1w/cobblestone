add_library(infra STATIC
    src/serialization/markdown_serializer.cpp
    src/storage/markdown_storage.cpp
)

target_include_directories(infra PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(infra PUBLIC core)
