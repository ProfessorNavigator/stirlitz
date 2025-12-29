cmake_minimum_required(VERSION 3.16)

project(stirlitz VERSION 1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CREATE_HTML_DOCS "Build html documentation" OFF)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
if(BUILD_SHARED_LIBS)
  add_library(stirlitz SHARED)
  set_target_properties(stirlitz PROPERTIES
      VERSION ${PROJECT_VERSION}
      SOVERSION ${PROJECT_VERSION_MAJOR}
    )
else()
  add_library(stirlitz STATIC)
endif()

set_target_properties(stirlitz PROPERTIES POSITION_INDEPENDENT_CODE True)

add_subdirectory(include)
add_subdirectory(src)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GCRYPT REQUIRED IMPORTED_TARGET libgcrypt)
pkg_check_modules(GPG-ERROR REQUIRED IMPORTED_TARGET gpg-error)

target_include_directories(stirlitz
  PRIVATE include
)

target_link_libraries(stirlitz
  PUBLIC PkgConfig::GCRYPT
  PUBLIC PkgConfig::GPG-ERROR
)

if(CMAKE_SYSTEM_NAME MATCHES "Android")
  find_package(Intl REQUIRED)
  target_link_libraries(stirlitz
      PUBLIC Intl::Intl
  )
endif()

if(CREATE_HTML_DOCS)
  find_package(Doxygen REQUIRED OPTIONAL_COMPONENTS dot)
endif()

if(DOXYGEN_FOUND)
  set(DOXYGEN_GENERATE_HTML YES)
  set(DOXYGEN_SHOW_USED_FILES NO)
  set(DOXYGEN_SHOW_FILES NO)

  doxygen_add_docs(documentation
    ALL
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/include"
    COMMENT "Generating ${PROJECT_NAME} html documentation"
  )
endif()

include(GNUInstallDirs)

install(TARGETS stirlitz EXPORT "${PROJECT_NAME}Targets"
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(DIRECTORY include/
  EXPORT "${PROJECT_NAME}Targets"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
  FILES_MATCHING PATTERN "*.h"
)

install(EXPORT "${PROJECT_NAME}Targets"
  NAMESPACE "${PROJECT_NAME}::"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

if(CREATE_HTML_DOCS)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/html
  )
endif()

include(CMakePackageConfigHelpers)

write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${mlbookproc_VERSION}
    COMPATIBILITY AnyNewerVersion)

configure_package_config_file(${PROJECT_NAME}Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)
