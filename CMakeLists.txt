cmake_minimum_required(VERSION 3.1)

project(quicklist-client)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_THREAD_PREFER_PTHREAD true)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(TARGET quicklist-client)

find_package(Threads REQUIRED)
find_package(Boost 1.78.0 COMPONENTS log json REQUIRED)

find_library(CRYPTO NAMES crypto SHARED)
find_library(SSL NAMES ssl SHARED)

add_compile_options(-Wall -Wextra -pedantic)
ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)


file (GLOB APP_FILES
    main.cpp
    websocketsession.cpp
    websocketsession.h
    unixdomainserver.cpp
    unixdomainserver.h
    unixdomainsession.cpp
    unixdomainsession.h
    quicklistclient.cpp
    quicklistclient.h
    namespace.cpp
    namespace.h
    log.cpp
    log.h
)

source_group ("" FILES ${APP_FILES})

add_executable(${TARGET} ${APP_FILES})

target_link_libraries(${TARGET} Threads::Threads ${Boost_LIBRARIES} ${CRYPTO} ${SSL})

install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
