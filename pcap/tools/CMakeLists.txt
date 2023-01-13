add_library(pe-pcap-tools STATIC
            ippacket.cpp
            ostream.cpp)

target_link_libraries(pe-pcap-tools
                     #pcap
                     )

