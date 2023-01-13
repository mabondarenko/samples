add_library(pe-curl STATIC
            easy.cpp
            engine.cpp
            downloader.cpp
            httprequest.cpp
            jsonrpc.cpp
            multi.cpp
            remotedir.cpp
            service.cpp
            syncwait.cpp
            tcpsocket.cpp
            transfer.cpp
            udpsocket.cpp
            url.cpp)

#add_definitions(-D_CURL_DEBUG)

add_library(curl-ssh2 STATIC IMPORTED)
set_target_properties(curl-ssh2 PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/contrib/usr/lib/libcurl.a")

target_link_libraries(pe-curl
                      boost_chrono
                      boost_filesystem
                      boost_system
                      curl-ssh2
                      pthread
                      ssh2
                      ssl
                      crypto
                      z)
