PROJECT (CoProcessing)

SET(CoProcessing_VERSION_MAJOR "0")
SET(CoProcessing_VERSION_MINOR "1")
SET(CoProcessing_VERSION_PATCH "0")

IF (NOT EXECUTABLE_OUTPUT_PATH)
  SET(EXECUTABLE_OUTPUT_PATH ${CoProcessing_BINARY_DIR}/bin CACHE INTERNAL 
    "Output directory for coprocessing executables.")
ENDIF (NOT EXECUTABLE_OUTPUT_PATH)

IF (NOT LIBRARY_OUTPUT_PATH)
  SET(LIBRARY_OUTPUT_PATH ${CoProcessing_BINARY_DIR}/bin CACHE INTERNAL 
    "Output directory for coprocessing libraries.")
ENDIF (NOT LIBRARY_OUTPUT_PATH)

SET (CoProcessing_TEST_DIR "${CoProcessing_BINARY_DIR}/Testing/Temporary")

#--------------------------------------------------
# Find and Use ParaView
#--------------------------------------------------
IF (ParaView_SOURCE_DIR)
  INCLUDE_DIRECTORIES(
    ${PARAVIEW_INCLUDE_DIRS}
    ${PARAVIEW_GUI_INCLUDE_DIRS}
    ${PARAVIEW_KWSYS_INCLUDE_DIRS}
    ${VTK_INCLUDE_DIR}
  )
ELSE (ParaView_SOURCE_DIR)
  FIND_PACKAGE(ParaView REQUIRED)
  INCLUDE(${PARAVIEW_USE_FILE})
ENDIF (ParaView_SOURCE_DIR)

ADD_SUBDIRECTORY(CoProcessor)
OPTION(BUILD_COPROCESSING_ADAPTORS "Build CoProcessing Adaptors." OFF)
MARK_AS_ADVANCED(BUILD_COPROCESSING_ADAPTORS)
IF(BUILD_COPROCESSING_ADAPTORS)
  ADD_SUBDIRECTORY(Adaptors)
ENDIF (BUILD_COPROCESSING_ADAPTORS)

SET(CoProcessing_USE_FILE 
  "${CoProcessing_SOURCE_DIR}/CoProcessingUse.cmake")

SET(CoProcessing_LIBRARY_DIRS
  ${LIBRARY_OUTPUT_PATH}
  )

SET(CoProcessing_INCLUDE_DIRS
  ${CoProcessing_SOURCE_DIR}/Adaptors/PythonAdapter
  ${CoProcessing_SOURCE_DIR}/Adaptors/FortranAdaptors
  ${CoProcessing_SOURCE_DIR}/CoProcessor
  )

# Save library dependencies.
EXPORT_LIBRARY_DEPENDENCIES(
  ${CoProcessing_BINARY_DIR}/CPLibraryDepends.cmake)

# Perform configuration.
CONFIGURE_FILE(${CoProcessing_SOURCE_DIR}/CoProcessingConfig.cmake.in
  ${CoProcessing_BINARY_DIR}/CoProcessingConfig.cmake @ONLY IMMEDIATE)

