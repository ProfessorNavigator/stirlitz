cmake_minimum_required(VERSION 3.19)
project(Stirlitz VERSION 1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Threads REQUIRED)

qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES ru)

qt_add_executable(Stirlitz)
qt_add_resources(Stirlitz "translations"
    PREFIX "/translations"
    BASE "${CMAKE_SOURCE_DIR}/qm"
    FILES "${CMAKE_SOURCE_DIR}/qm/Stirlitz_ru.qm"
)

add_subdirectory(include)
add_subdirectory(src)

add_subdirectory(stirlitz libstirlitz)

target_include_directories(Stirlitz
    PRIVATE include
    PRIVATE stirlitz/include
)

target_link_libraries(Stirlitz
    PRIVATE Qt::Core
    PRIVATE Qt::Widgets
    PRIVATE stirlitz
    PRIVATE Threads::Threads
)

include(GNUInstallDirs)

install(TARGETS Stirlitz
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    install(FILES ru.mail.bobilev_yury.Stirlitz.desktop
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
    install(FILES Stirlitz.png
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps")
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set_property(TARGET Stirlitz PROPERTY WIN32_EXECUTABLE true)
    install(FILES Stirlitz.ico
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps")
endif()
