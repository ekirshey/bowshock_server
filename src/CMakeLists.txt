include_directories(
	${BEAST_INCLUDE_DIR}
	${ASIO_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src
)
        
file( GLOB SRC 
    #INCLUDE
    ${PROJECT_SOURCE_DIR}/src/*.h

    #SOURCE
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_executable(bowshock_server
    ${SRC}
)

add_compile_definitions(_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING)
add_compile_definitions(_WIN32_WINNT=0x0600)

target_link_libraries( bowshock_server PRIVATE nlohmann_json 
											   nlohmann_json::nlohmann_json )

set_property(TARGET bowshock_server PROPERTY CXX_STANDARD 17)