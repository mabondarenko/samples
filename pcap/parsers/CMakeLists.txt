add_library(pe-pcap-parsers STATIC
            datalink.cpp
            factory.cpp
            ip.cpp
            message.cpp
            raw.cpp
            smart.cpp
            tcp.cpp
            tcpreassembler.cpp
            udp.cpp
            voip.cpp
            )

add_library(PcapCommon++ STATIC IMPORTED)
set_target_properties(PcapCommon++ PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/contrib/usr/lib/libCommon++.a")

add_library(PcapPacket++ STATIC IMPORTED)
set_target_properties(PcapPacket++ PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/contrib/usr/lib/libPacket++.a")

#add_library(Pcap++ STATIC IMPORTED)
#set_target_properties(Pcap++ PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/contrib/usr/lib/libPcap++.a")

target_link_libraries(pe-pcap-parsers
                      PcapPacket++
                      PcapCommon++
                      snappy)
