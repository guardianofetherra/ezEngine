﻿ez_cmake_init()

ez_build_filter_everything()

ez_requires(EZ_CMAKE_PLATFORM_WINDOWS_UWP)

ez_requires(EZ_ENABLE_MIXEDREALITY_SUPPORT)

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ez_create_target(LIBRARY ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  RendererCore
)