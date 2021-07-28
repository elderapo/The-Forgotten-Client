cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# apt install build-essential git
# git clone https://github.com/Kitware/CMake/; cd CMake
# ./bootstrap && make && sudo make install

# cat ~/.hunter/_Base/Download/Hunter/<version>/<hash>/Unpacked/cmake/projects/SDL2/hunter.cmake
# hunter_add_version(
#         PACKAGE_NAME
#         SDL2
#         VERSION
#         "2.0.8"
#         URL
#         "https://www.libsdl.org/release/SDL2-2.0.8.tar.gz"
#         SHA1
#         959ec6ded277821995d2e3cafd973f6ab9f48ab7
# )

# *****************************************************************************
# Cmake Hunter
# *****************************************************************************
if(UNIX)
  include("cmake/HunterGate.cmake")

  HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.288.tar.gz"
    SHA1 "6c9b2bc606d86ae31f96a62fc68f0a593024815b"
    LOCAL
  )

endif()

# *****************************************************************************
# Project The Forgotten Client
# *****************************************************************************
project(the-forgotten-client)


# *****************************************************************************
# Options
# *****************************************************************************
option(OPTIONS_ENABLE_CCACHE "Enable ccache" ON)
option(OPTIONS_ENABLE_IPO "Check and Enable interprocedural optimization (IPO/LTO)" ON)


# *****************************************************************************
# Append cmake search path
# *****************************************************************************
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


# *****************************************************************************
# Include cmake tools
# *****************************************************************************
include(MessageColors)
include(LoggingHelper)


# *****************************************************************************
# Options Code
# *****************************************************************************

# === CCACHE ===
if(OPTIONS_ENABLE_CCACHE)
    find_program(CCACHE ccache)
    if(CCACHE)
        log_option_enabled("ccache")
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
        else()
        log_option_disabled("ccache")
    endif()
endif()


# === IPO ===
option(OPTIONS_ENABLE_IPO "Check and Enable interprocedural optimization (IPO/LTO)" ON)
if(OPTIONS_ENABLE_IPO)
  log_option_enabled("ipo")

  include(CheckIPOSupported)
  check_ipo_supported(RESULT result OUTPUT output)
  if(result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    log_war("IPO is not supported: ${output}")
  endif()
else()
  log_option_disabled("ipo")
endif()


# *****************************************************************************
# Add project
# *****************************************************************************
add_subdirectory(src)
