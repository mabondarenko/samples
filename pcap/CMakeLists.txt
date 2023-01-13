add_subdirectory(grabber)
add_subdirectory(parsers)
add_subdirectory(tools)

add_library(pe-pcap STATIC
            core.cpp)

target_link_libraries(pe-pcap
                      pe-context
                      pe-io
                      pe-pcap-grabber)

