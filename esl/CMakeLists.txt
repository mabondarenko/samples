add_library(pe-esl STATIC
            client.cpp
            event.cpp)

target_link_libraries(pe-esl
                      boost_chrono
                      esl)
