add_library(pe-redis STATIC
            pool.cpp
            pipeline.cpp
            redis.cpp
            script.cpp)

target_link_libraries(pe-redis
                      redis3m)
