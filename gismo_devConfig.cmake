### G+SMO CMake Configuration file
## 
## Author: Angelos Mantzaflaris 
## Copyright (C) 2014 - RICAM-Linz.
######################################################################
##
## Config file for the G+SMO library
## It defines the following variables
## GISMO_VERSION      - version string of G+SMO
## GISMO_INCLUDE_DIRS - include directories for G+SMO
## GISMO_LIBRARIES    - libraries to link against
## GISMO_EXECUTABLE   - none 
##
######################################################################

# Compute paths
get_filename_component(GISMO_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

# Our library dependencies (contains definitions for IMPORTED targets)
#if(NOT TARGET gismo AND NOT gismo_BINARY_DIR)
#  include("${GISMO_CMAKE_DIR}/gismoTargets.cmake")
#endif()

# These are IMPORTED targets created by gismoTargets.cmake (add TARGETS gsView ...)

#set(GISMO_VERSION        "0.8.5")
#set(GISMO_VERSION_MAJOR  "0")
#set(GISMO_VERSION_MINOR  "8")
#set(GISMO_VERSION_BUILD  "")

set(GISMO_DEV_INCLUDE_DIRS "/home/samwise/weinmueller/gismo/extensions/unsupported/src;/home/samwise/weinmueller/gismo/extensions/unsupported/external;/home/samwise/weinmueller/gismo/extensions/unsupported/extensions;/samwise/home/weinmueller/gismo/extensions/unsupported;/home/samwise/weinmueller/gismo/extensions/unsupported/src;/home/samwise/weinmueller/gismo/extensions/unsupported/external;/samwise/home/weinmueller/gismo/extensions/unsupported")
set(GISMO_DEV_LIBRARY_DIR  "/samwise/home/weinmueller/gismo/lib")
set(GISMO_DEV_LIBRARIES    gismo_dev)
#set(GISMO_MODULES_DIR "/samwise/home/weinmueller/gismo/modules")
#set(GISMO_CMAKE_DIRS   "")

#set(gismo_USE_FILE "")
#message("gismo_USE_FILE")
