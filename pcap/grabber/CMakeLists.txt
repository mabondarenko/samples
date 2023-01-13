add_library(pe-pcap-grabber STATIC
            capture.cpp
            config.cpp
            device.cpp
            grabber.cpp
            sniffer.cpp)

target_link_libraries(pe-pcap-grabber
                      boost_system
                      pcap
                      pe-livecapture
                      pe-pcap-parsers
                      pe-sockets)

