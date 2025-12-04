add_executable(client)
set_property(TARGET client PROPERTY CXX_STANDARD 17)
enable_warnings(client)
target_sources(client PRIVATE "main.cpp")
