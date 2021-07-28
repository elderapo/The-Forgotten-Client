project(the-forgotten-client)

# *****************************************************************************
# Cmake Features
# *****************************************************************************
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Global gives error when building spdlog
# Avoid changes by cmake/make on the source tree
# https://gitlab.kitware.com/cmake/cmake/issues/18403
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Make will print more details
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# *****************************************************************************
# Options
# *****************************************************************************
option(OPTIONS_ENABLE_OPENMP "Enable Open Multi-Processing support." ON)


# *****************************************************************************
# Client
# *****************************************************************************
add_executable(${PROJECT_NAME} "")


# *****************************************************************************
# Options Code
# *****************************************************************************


# === OpenMP ===
if(OPTIONS_ENABLE_OPENMP)
  log_option_enabled("openmp")
  find_package(OpenMP)
  if(OpenMP_CXX_FOUND)
      target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
  endif()
else()
  log_option_disabled("openmp")
endif()

# === IPO ===
check_ipo_supported(RESULT result OUTPUT output)
if(result)
  set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(WARNING "IPO is not supported: ${output}")
endif()

# *****************************************************************************
# Packages / Libs
# *****************************************************************************

hunter_add_package(ZLIB)
find_package(ZLIB CONFIG REQUIRED)

# hunter_add_package(SDL2)
# find_package(SDL2 CONFIG REQUIRED)

hunter_add_package(CURL)
find_package(CURL CONFIG REQUIRED)

hunter_add_package(lzma)
find_package(lzma CONFIG REQUIRED)

hunter_add_package(Vulkan-Headers)
find_package(Vulkan-Headers CONFIG REQUIRED)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GMP REQUIRED)

include(GNUInstallDirs)


# *****************************************************************************
# Library Configuration
# *****************************************************************************
target_sources(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/animatedText.cpp
    ${CMAKE_CURRENT_LIST_DIR}/animator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/automap.cpp
    ${CMAKE_CURRENT_LIST_DIR}/config.cpp
    ${CMAKE_CURRENT_LIST_DIR}/connection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/container.cpp
    ${CMAKE_CURRENT_LIST_DIR}/creature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/distanceEffect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/effect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/engine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/game.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/AdvancedGraphicsOptions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/BattleWindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/Buttons.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/Channels.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/CharacterList.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/Chat.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/ConsoleOptions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/ContainerWindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/DeathNotification.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/EnterGame.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/ExitWarning.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/GeneralOptions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/GraphicsOptions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/Health.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/Help.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/Hotkeys.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/IgnoreList.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/Info.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/Inventory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/ItemMove.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/itemUI.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/Logout.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/MainWindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/Messagebox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/Minimap.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/ModalDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/Motd.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/Options.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/Outfits.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/ProtocolOptions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/Quests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/ReadWrite.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/RuleViolations.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/ServerBrowser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/ShopWindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/SkillsWindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/SpellsWindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/TradeWindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/TutorialHint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI/VipWindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_Button.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_CharacterView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_CheckBox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_Console.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_Container.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_Content.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_ContextMenu.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_Element.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_Flash.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_Grouper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_Icon.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_Label.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_ListBox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_Log.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_MultiTextBox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_Panel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_PanelWindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_ScrollBar.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_Separator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_StaticImage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_Description.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_TextBox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GUI_Elements/GUI_Window.cpp
    ${CMAKE_CURRENT_LIST_DIR}/http.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inputMessage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/item.cpp
    ${CMAKE_CURRENT_LIST_DIR}/json/json.cpp
    ${CMAKE_CURRENT_LIST_DIR}/light.cpp
    ${CMAKE_CURRENT_LIST_DIR}/lzma/Alloc.c
    ${CMAKE_CURRENT_LIST_DIR}/lzma/LzFind.c
    # ${CMAKE_CURRENT_LIST_DIR}/lzma/LzFindMt.c
    ${CMAKE_CURRENT_LIST_DIR}/lzma/LzmaDec.c
    ${CMAKE_CURRENT_LIST_DIR}/lzma/LzmaEnc.c
    ${CMAKE_CURRENT_LIST_DIR}/lzma/LzmaLib.c
    # ${CMAKE_CURRENT_LIST_DIR}/lzma/Threads.c
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/map.cpp
    ${CMAKE_CURRENT_LIST_DIR}/outputMessage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/protocol.cpp
    ${CMAKE_CURRENT_LIST_DIR}/protocolgame.cpp
    ${CMAKE_CURRENT_LIST_DIR}/protocollogin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/protocolloginHttp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rsa.cpp
    ${CMAKE_CURRENT_LIST_DIR}/screenText.cpp
    ${CMAKE_CURRENT_LIST_DIR}/softwareDrawning.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spriteManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/staticText.cpp
    ${CMAKE_CURRENT_LIST_DIR}/surfaceDirect3D11.cpp
    ${CMAKE_CURRENT_LIST_DIR}/surfaceDirect3D9.cpp
    ${CMAKE_CURRENT_LIST_DIR}/surfaceDirectDraw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/surfaceOpengl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/surfaceOpenglCore.cpp
    ${CMAKE_CURRENT_LIST_DIR}/surfaceOpengles.cpp
    ${CMAKE_CURRENT_LIST_DIR}/surfaceOpengles2.cpp
    ${CMAKE_CURRENT_LIST_DIR}/surfaceSoftware.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/surfaceVulkan.cpp
    ${CMAKE_CURRENT_LIST_DIR}/thing.cpp
    ${CMAKE_CURRENT_LIST_DIR}/thingManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/util.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
    ${GMP_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    SDL2::SDL2-static
    CURL::libcurl
    lzma::lzma
    Vulkan-Headers::Vulkan-Headers
    OpenGL::OpenGL
    OpenGL::GLX
    OpenGL::EGL
    ZLIB::ZLIB
    ${GMP_LIBRARIES}
)

target_compile_options(${PROJECT_NAME} PUBLIC -fpermissive -march=native -mfma4)

# Force cpu family
# target_compile_options(${PROJECT_NAME} PUBLIC -fpermissive -march=skylake -mfma4)

# print all variables
# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

set_target_properties(${PROJECT_NAME}
    PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
