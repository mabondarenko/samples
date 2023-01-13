add_library(pe-livecapture STATIC
            connection.cpp
            grabber.cpp
            server.cpp
            servertcp.cpp
            serverws.cpp
            srvfactory.cpp)

target_link_libraries(pe-livecapture
                      boost_system
                      pe-sockets)

