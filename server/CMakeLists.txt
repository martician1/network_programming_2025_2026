add_executable(server)
set_property(TARGET server PROPERTY CXX_STANDARD 17)
enable_warnings(client)
target_sources(server PRIVATE "main.cpp")
