CPMAddPackage(

  NAME SDL
  GITHUB_REPOSITORY libsdl-org/SDL 
  VERSION 2.30.1
  GIT_TAG  main OPTIONS
  "BUILD_SHARED_LIBS OFF"

  )

# CPMAddPackage(
#   NAME SDL_mixer
#   VERSION 3.0.0
#     GIT_TAG main
#   GITHUB_REPOSITORY libsdl-org/SDL_mixer    "BUILD_SHARED_LIBS OFF"
#   )
CPMAddPackage(

  NAME FTXUI
  VERSION 5.0.0
  GITHUB_REPOSITORY ArthurSonzogni/FTXUI
  GIT_TAG cdf2890
  OPTIONS
  "FTXUI_BUILD_EXAMPLES off"
	"FTXUI_ENABLE_INSTALL off"
	"FTXUI_BUILD_TESTS off"

)

set(EXECUTABLE_NAME e)
include_directories($ENV{CPLUS_INCLUDE_PATH}
  ${CMAKE_SOURCE_DIR}/include
  )
add_library(AudioPlayer AudioPlayer.cpp)
add_library(SonicUI SonicUi.cpp)

set_target_properties(AudioPlayer PROPERTIES CXX_STANDARD 17)
set_target_properties(SonicUI PROPERTIES CXX_STANDARD 17)
target_link_libraries(AudioPlayer PRIVATE
  SDL3::SDL3
  PRIVATE "$ENV{C_LIBRARY_PATH}\\SDL3_mixer.lib"
)
# target_link_libraries(AudioPlayer PRIVATE SDL3::SDL3 PRIVATE SDL3_mixer::SDL3_mixer)
target_link_libraries(SonicUI PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  )
add_executable(${EXECUTABLE_NAME} main.cpp)
set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(${EXECUTABLE_NAME}   PRIVATE SonicUI
  PRIVATE AudioPlayer
  PRIVATE ftxui::component
  )
# set_target_properties(C
