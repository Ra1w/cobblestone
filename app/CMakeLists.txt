add_library(app STATIC
    src/commands/add_entity_command.cpp
    src/commands/delete_entity_command.cpp
    src/commands/move_entity_command.cpp
    src/commands/edit_entity_command.cpp
    src/commands/command_manager.cpp
    src/life_engine.cpp
)

target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(app PUBLIC core)
