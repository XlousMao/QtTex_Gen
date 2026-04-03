cmake_minimum_required(VERSION 3.16)

project(QtTex_Gen VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appQtTex_Gen
    main.cpp
    core/core.h
    core/AnkiCard.cpp
    core/AnkiCard.h
    core/CardParser.cpp
    core/CardParser.h
    core/FileHander.cpp
    core/FileHander.h
    core/LatexConverter.cpp
    core/LatexConverter.h
    controller/AppController.cpp
    controller/AppController.h
)

qt_add_qml_module(appQtTex_Gen
    URI QtTex_Gen
    QML_FILES
        ui/MainWindow.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appQtTex_Gen PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appQtTex_Gen
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appQtTex_Gen
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appQtTex_Gen
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
