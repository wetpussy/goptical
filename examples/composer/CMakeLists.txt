set(SOURCES
  composer.cpp
  )

find_package( OpenCV REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS})

get_filename_component( FNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_executable(${FNAME} ${SOURCES})
target_link_libraries(${FNAME} ${PROJECT_NAME}_static ${OpenCV_LIBRARIES})
