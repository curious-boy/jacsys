cmake_minimum_required(VERSION 2.6)

project(jacSystem CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "debug")
endif()

if(NOT MUDUO_PATH)
  set(MUDUO_PATH "/home/$ENV{USER}/build/${CMAKE_BUILD_TYPE}-install")
endif()

set(CXX_FLAGS
 -g
 # -DVALGRIND
 # -DMUDUO_STD_STRING
  -Wall
 -Wextra
-fpermissive
 # -m32
 # -Werror
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -march=native
 # -MMD
 # -std=c++0x
 -rdynamic
 )
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(MUDUO_PATH "/home/admin/frankz/github/build/release-install")
#set(MUDUO_PATH "/mnt/hgfs/github/jacServer/build/release-install")


find_path(Muduo_INCLUDE_DIR muduo "${MUDUO_PATH}/include")
find_path(Muduo_LIBRARY_DIR libmuduo_net.a "${MUDUO_PATH}/lib")

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${Muduo_LIBRARY_DIR})
message(STATUS ${Muduo_INCLUDE_DIR})
message(STATUS ${Muduo_LIBRARY_DIR})

include_directories(${Muduo_INCLUDE_DIR})
find_library(muduo_base muduo_base)
find_library(muduo_net muduo_net)
message(STATUS ${muduo_base})
message(STATUS ${muduo_net})


# need change for the real path
set(MYSQL_PATH "/usr")
include_directories(${MYSQL_PATH}/include)
link_directories(${MYSQL_PATH}/lib)

# need change for the real path
set(MYSQLPP_PATH "/usr/local")
include_directories(${MYSQLPP_PATH}/include)
link_directories(${MYSQLPP_PATH}/lib)


find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR})

add_executable(jacServer ../src/jacServer.cc ../src/database_operator.cc)
target_link_libraries(jacServer ${muduo_net})
target_link_libraries(jacServer ${muduo_base})
target_link_libraries(jacServer pthread rt mysqlclient mysqlpp)

#add_executable(jacClient ../src/jacClient.cc )

target_link_libraries(jacClient ${muduo_net})
target_link_libraries(jacClient ${muduo_base})
target_link_libraries(jacClient pthread rt)
