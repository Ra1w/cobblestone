if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(app_tests 
    core/id_test.cpp
    core/registry_test.cpp
    core/tag_test.cpp
    core/timestamp_test.cpp
    core/title_test.cpp
    infra/serializer_test.cpp
    app/life_engine_test.cpp
)

target_include_directories(app_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core/include
    ${CMAKE_SOURCE_DIR}/app/include
    ${CMAKE_SOURCE_DIR}/infra/include
)

target_link_libraries(app_tests PRIVATE
    app
    core
    infra
    gmock
    gmock_main
    gtest
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(app_tests)
