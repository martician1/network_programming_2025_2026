cmake_minimum_required(VERSION 3.15)

project(NetworkProgrammingAssignment LANGUAGES CXX)

function(enable_warnings target)
    if (MSVC)
        target_compile_options(${target} PRIVATE
            /W4
            /permissive-
        )
    else()
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic
            -Wshadow -Wformat=2 -Wundef
            -Wconversion -Wsign-conversion
            -Wduplicated-cond -Wduplicated-branches
            -Wlogical-op -Wnull-dereference
            -Wuseless-cast -Wdouble-promotion
        )
    endif()
endfunction()

add_subdirectory(client)
add_subdirectory(server)
