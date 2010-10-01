##
# Locate NIScope 
#
# REQUIRED ENVIRONMENT VARIABLES
# ------------------------------
#
# NIEXTCCOMPILERSUPP (windows)
#
# OUTPUT VARIABLES
# ----------------
# NISCOPE_LIBRARIES
# NISCOPE_INCLUDE_DIR
# NISCOPE_FOUND
#
# NISCOPE_NISCOPE_LIBRARY
# NISCOPE_IVI_LIBRARY
# NISCOPE_NIMODINST_LIBRARY
#
FUNCTION(_NISCOPE_ASSERT _VAR _MSG)
IF(NOT ${_VAR})
  IF(${NISCOPE_FIND_REQUIRED})
    MESSAGE(FATAL_ERROR ${_MSG}) 
  ELSE()
    MESSAGE(STATUS ${_MSG}) 
  ENDIF()
ENDIF()
ENDFUNCTION(_NISCOPE_ASSERT)

#
set(NISCOPE_FOUND "NO")

# includes
find_path(NISCOPE_INCLUDE_DIR niscope.h
    HINTS
    $ENV{VXIPNPPATH}/win95/
    $ENV{VXIPNPPATH}/winnt/
    PATH_SUFFIXES include
)
#message("NISCOPE_INCLUDE_DIR is ${NISCOPE_INCLUDE_DIR}")
_NISCOPE_ASSERT(NISCOPE_INCLUDE_DIR "Could not find niscope.h")
      
# libs
find_library(NISCOPE_NISCOPE_LIBRARY niscope.lib# ivi.lib niModInst.lib
  HINTS
  $ENV{VXIPNPPATH}/win95/
  $ENV{VXIPNPPATH}/winnt/
  PATH_SUFFIXES lib/msc
  )
#message("NISCOPE_NISCOPE_LIBRARY is ${NISCOPE_NISCOPE_LIBRARY}")
_NISCOPE_ASSERT(NISCOPE_NISCOPE_LIBRARY "Could not find niscope.lib")

find_library(NISCOPE_IVI_LIBRARY ivi.lib
  HINTS
  $ENV{VXIPNPPATH}/win95/
  $ENV{VXIPNPPATH}/winnt/
  PATH_SUFFIXES lib/msc
  )
#message("NISCOPE_IVI_LIBRARY is ${NISCOPE_IVI_LIBRARY}")
_NISCOPE_ASSERT(NISCOPE_IVI_LIBRARY "Could not find ivi.lib")

find_library(NISCOPE_NIMODINST_LIBRARY niModInst.lib 
  HINTS
  $ENV{VXIPNPPATH}/win95/
  $ENV{VXIPNPPATH}/winnt/
  PATH_SUFFIXES lib/msc
  )
#message("NISCOPE_NIMODINST_LIBRARY is ${NISCOPE_NIMODINST_LIBRARY}")
_NISCOPE_ASSERT(NISCOPE_NIMODINST_LIBRARY "Could not find niModInst.lib")

set(NISCOPE_LIBRARIES
    ${NISCOPE_NISCOPE_LIBRARY}
    ${NISCOPE_IVI_LIBRARY}
    ${NISCOPE_NIMODINST_LIBRARY}
   )
set(NISCOPE_FOUND "YES")
