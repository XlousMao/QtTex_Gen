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

# 自动调用 windeployqt 将所需的 Qt 运行库和插件拷贝到 exe 所在目录
if(WIN32)
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET appQtTex_Gen POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}" --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/ui" "$<TARGET_FILE:appQtTex_Gen>"
            COMMENT "Running windeployqt to copy Qt dependencies..."
        )
    endif()
endif()

include(GNUInstallDirs)
install(TARGETS appQtTex_Gen
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
