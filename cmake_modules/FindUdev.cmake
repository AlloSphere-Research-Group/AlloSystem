# - Try to find Udev
# Once done this will define
#
#  UDEV_FOUND - system has Udev
#  UDEV_INCLUDE_DIR - the Udev include directory
#  UDEV_LIBRARY - Link these to use Udev
#

include(LibFindMacros)

# Dependencies
#libfind_package(LO Magick)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(UDEV_PKGCONF libudev)

# Include dir
find_path(UDEV_INCLUDE_DIR
  NAMES libudev.h
  PATHS ${UDEV_PKGCONF_INCLUDE_DIRS} /usr/include /usr/local/include /opt/local/include
)

# Finally the library itself
find_library(UDEV_LIBRARY
  NAMES udev
  PATHS ${UDEV_PKGCONF_LIBRARY_DIRS}/usr/lib /usr/local/lib /opt/local/lib
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(UDEV_PROCESS_INCLUDES UDEV_INCLUDE_DIR)
set(UDEV_PROCESS_LIBS UDEV_LIBRARY)
libfind_process(UDEV)



