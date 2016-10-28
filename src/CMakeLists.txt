cmake_minimum_required(VERSION 2.8)

project( OcTree )
find_package( ITK REQUIRED )
find_package( awesomeIO REQUIRED)
find_package( OpenCV REQUIRED)
#find_package( cereal CONFIG REQUIRED)
find_package( Boost REQUIRED COMPONENTS system filesystem)

include( ${ITK_USE_FILE} )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ./)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(ITK_DIR /data/PLI-Core/Schubert/toolkits/ITK/InsightToolkit-4.4.0_build)

add_definitions(--no-warnings)

include_directories ( ${ITK_INCLUDE_DIRS}
			${Boost_INCLUDE_DIRS}
			${Cereal_DIRS}
                        ${awesomeIO_INCLUDE_DIRS}
			${OpenCV_LIBS}
			${PROJECT_SOURCE_DIR}/../libs/cereal-1.2.1/include
			
                    )
add_definitions(-Wall)

add_executable( 
        OcTree
        octree.cpp
	ocmainskalar.cpp
        )
target_link_libraries( OcTree ${Boost_INCLUDE_DIRS}
			 ${ITK_LIBRARIES}			
			${awesomeIO_LIBS} 
			${OpenCV_LIBS}
			
			)

