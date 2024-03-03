cmake_minimum_required (VERSION 3.26)

add_executable(server
    "errors.cpp"
    "main.cpp"
    "server.cpp"
    "listener.cpp"
    "detail/listener.cpp"
    "http_session.cpp"
    "websocket_session.cpp")

target_link_libraries(server PUBLIC Boost::boost)

install(TARGETS server RUNTIME DESTINATION .)
