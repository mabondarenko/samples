add_library(pe-scgi STATIC
            connection.cpp
            dispatcher.cpp
            engine.cpp
            parser.cpp
            pool.cpp
            reqbase.cpp
            request.cpp
            response.cpp
            server.cpp)

target_link_libraries(pe-scgi
                      pe-sockets
                      pe-tools)
