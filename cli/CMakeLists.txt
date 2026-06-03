add_executable(cobblestone
  main.cpp
  src/console_app.cpp
)

target_include_directories(cobblestone PRIVATE include)

target_link_libraries(cobblestone PRIVATE app infra)
