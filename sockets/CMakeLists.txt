add_library(pe-sockets STATIC
            connection.cpp
            tcp/connection.cpp
            tcp/server.cpp
            udp/connection.cpp
            web/connection.cpp
            web/server.cpp)

target_link_libraries(pe-sockets)
