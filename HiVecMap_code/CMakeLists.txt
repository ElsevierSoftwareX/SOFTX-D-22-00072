cmake_minimum_required (VERSION 3.9)

project (HiVecMap
  VERSION 1.0.0
  LANGUAGES CXX
)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(PNG REQUIRED)
find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED)



if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS"${CMAKE_EXE_LINKER_FLAGS}${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(INCLUDE_DIR /usr/local/include)
set(CORE_LIBS   ${PROJECT_SOURCE_DIR}/bin/libhigis.a     ${PROJECT_SOURCE_DIR}/bin/libhigis.so)
set(REDIS_LIBS       /usr/local/lib/libhiredis.a    /usr/local/lib/libhiredis.so)

add_definitions(-std=c++11)
ADD_DEFINITIONS(-Wall -O3 -g)

##### 指定头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include 
                    ${INCLUDE_DIR} 
                    ${PNG_INCLUDE_DIRS}
                    ${MPI_INCLUDE_PATH}
                    )



add_executable(buildIndex  
                                  ${PROJECT_SOURCE_DIR}/src/buildIndex_main.cpp
                                  ${PROJECT_SOURCE_DIR}/src/VPQTreeNode.cpp 
                                  ${PROJECT_SOURCE_DIR}/src/buildVPQTree.cpp
                                  )
#### 添加链接库
target_link_libraries(buildIndex ${CORE_LIBS})


add_executable(plotting  
                                  ${PROJECT_SOURCE_DIR}/src/plotting_main.cpp
                                  ${PROJECT_SOURCE_DIR}/src/VPQTreeNode.cpp 
                                  ${PROJECT_SOURCE_DIR}/src/VPQTreeNodeS.cpp
                                  ${PROJECT_SOURCE_DIR}/src/plotTile.cpp
                                  )
#### 添加链接库
target_link_libraries(plotting ${CORE_LIBS} ${REDIS_LIBS} ${PNG_LIBRARIES} ${MPI_LIBRARIES} -lmpich -lpthread)


add_executable(tile_server  
                                  ${PROJECT_SOURCE_DIR}/src/TMS_server.cpp
                                  )
#### 添加链接库
target_link_libraries(tile_server ${CORE_LIBS} ${PNG_LIBRARIES} ${REDIS_LIBS})







