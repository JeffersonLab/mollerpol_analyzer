cmake_minimum_required(VERSION 3.5)

project(PoddSDK LANGUAGES CXX)

#----------------------------------------------------------------------------
# Names of source files and target libraries.
# You do want to modify this section.

# The name of your package.
# The generated shared library will be named lib$(PACKAGE).so/dylib
set(PACKAGE MollerPol)

# 1. Sources (Pure implementation files only)
set(src
  MollerPolCalorimeter.cxx
  MollerPolApparatus.cxx
  Scaler9250.cxx
  ScalerTI.cxx
  Scaler3820.cxx
  MollerPolScalerEvtHandler.cxx
  MollerPolHelicityDecoderBoard.cxx
  MollerPolHelicityDecoder.cxx
  HelicityDecoderModule.cxx
  #--------GEM content--------
  gem/MPDModule.cxx
  gem/MollerPolGEMData.cxx
  gem/MollerPolGEMElement.cxx
  gem/MollerPolGEMManager.cxx
  gem/MollerPolGEMModule.cxx
  gem/MollerPolGEMTrackerBase.cxx
  gem/MollerPolGEMTrackerBase.cxx
  gem/MollerPolGEMSpectrometerTracker.cxx
  gem/MollerPolGEMSpectrometer.cxx
  )

# 2. Convert source names to headers safely (Bypasses MollerPolDict.h generation)
string(REPLACE .cxx .h headers "${src}")

# 3. Safe Injection: Add the generated dictionary to source tracking list AFTER headers are built
list(APPEND src "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}Dict.cxx")

set(allheaders ${headers})

#------------------------------------------------------------------------------
# Do not change anything below here unless you know what you are doing

#----------------------------------------------------------------------------
# Find Podd, if necessary, and load its CMake module
if(NOT TARGET Podd::HallA)
  if(DEFINED ENV{ANALYZER})
    set(CMAKE_PREFIX_PATH "$ENV{ANALYZER}" "${CMAKE_PREFIX_PATH}")
  endif()

  find_package(Podd 1.7 REQUIRED)
endif()
include(PoddCMakeEnv)

set_diagnostic_flags(WALL WEXTRA)

#----------------------------------------------------------------------------
# Library
add_library(${PACKAGE} SHARED ${src} ${allheaders})

target_compile_options(${PACKAGE} PRIVATE ${${PROJECT_NAME_UC}_DIAG_FLAGS_LIST})

target_include_directories(${PACKAGE}
  PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/gem>
  )
target_link_libraries(${PACKAGE} PUBLIC Podd::HallA)

include(GNUInstallDirs)

install(TARGETS ${PACKAGE}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
install(FILES ${allheaders} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

#----------------------------------------------------------------------------
# ROOT dictionary
build_root_dictionary(${PACKAGE} ${headers}
  TARGETS ${PACKAGE}
  LINKDEF ${PACKAGE}_LinkDef.h
  )
