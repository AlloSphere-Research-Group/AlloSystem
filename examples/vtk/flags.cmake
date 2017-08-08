# set your VTK directory using:
set(VTK_DIR "/home/andres/Documents/src/vtk-build")

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
target_link_libraries("${APP_NAME}" ${VTK_LIBRARIES})

# For debugging, to see what has been found by cmake:
#message("include dirs:${VTK_INCLUDE_DIRS}")
#message("library dirs: ${VTK_LIBRARY_DIRS}")
#message("${VTK_LIBRARIES} found!!")
