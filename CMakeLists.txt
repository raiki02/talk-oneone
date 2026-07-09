cmake_minimum_required(VERSION 3.16)

project(Talk-Oneone VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Multimedia)
find_package(Qt6 REQUIRED COMPONENTS WebSockets)
find_package(Qt6 REQUIRED COMPONENTS QuickControls2)
find_package(Qt6 REQUIRED COMPONENTS Sql)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appTalk-Oneone
    main.cpp
)

qt_add_qml_module(appTalk-Oneone
    URI Talk-Oneone
    QML_FILES
        Main.qml
        Components/SideButton.qml
        Components/ChatMessageListModel.qml
        Pages/StatePage.qml
        Pages/SettingPage.qml
        Pages/ChatPage.qml
        js/addListModel.js
    SOURCES
        audiomgr.cpp audiomgr.h
        configmgr.cpp configmgr.h
        gateway.cpp gateway.h
        application.h application.cpp
        logger.h logger.cpp
        dbmanager.h dbmanager.cpp
    RESOURCES
        res.qrc
)

set_target_properties(appTalk-Oneone PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appTalk-Oneone
    PRIVATE Qt6::Quick
)
target_link_libraries(appTalk-Oneone PRIVATE Qt6::Core)
target_link_libraries(appTalk-Oneone PRIVATE Qt6::Multimedia)
target_link_libraries(appTalk-Oneone PRIVATE Qt6::WebSockets)
target_link_libraries(appTalk-Oneone PRIVATE Qt6::QuickControls2)
target_link_libraries(appTalk-Oneone PRIVATE Qt6::Sql)

include(GNUInstallDirs)
install(TARGETS appTalk-Oneone
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
