add_executable(gamificator
  main.cpp
  src/console_app.cpp
)

target_include_directories(gamificator PRIVATE include)

target_link_libraries(gamificator PRIVATE app infra)
