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

add_executable(websockets_test
    ${SRC}
)

target_link_libraries( websockets_test )