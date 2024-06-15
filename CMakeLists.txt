cmake_minimum_required(VERSION 3.28)
project(CloudHSE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(Boost_USE_MULTITHREADED ON)

include(C:/Users/LeadM/.vcpkg-clion/vcpkg/scripts/buildsystems/vcpkg.cmake)
include_directories(C:/Users/LeadM/.vcpkg-clion/vcpkg/installed/x64-windows/include)
include_directories(C:/Users/LeadM/.vcpkg-clion/vcpkg/installed/x64-windows/lib)
include_directories(C:/Users/LeadM/.vcpkg-clion/vcpkg/packages/tgbot-cpp_x64-windows/include)


find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)
find_package(CURL)

include_directories(C:/Users/LeadM/.vcpkg-clion/vcpkg/installed/x64-windows/include ${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIR})

if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    add_definitions(-DHAVE_CURL)
endif()


add_executable(CloudHSE main.cpp)

target_link_libraries(CloudHSE
        C:/Users/LeadM/.vcpkg-clion/vcpkg/installed/x64-windows/lib/TgBot.lib
        ${CMAKE_THREAD_LIBS_INIT}
        ${OPENSSL_LIBRARIES}
        ${Boost_LIBRARIES}
        ${CURL_LIBRARIES}
)