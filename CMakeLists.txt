cmake_minimum_required(VERSION 3.16)

project(SupervisionDashboard VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 REQUIRED COMPONENTS Quick Sql)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.10)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appSupervisionDashboard
    main.cpp
)
set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(appSupervisionDashboard
    URI SupervisionDashboard
    QML_FILES
        Main.qml
        SOURCES sensorsimulator.h sensorsimulator.cpp
        QML_FILES StatCard.qml
        QML_FILES LiveChart.qml
        QML_FILES GaugeWidget.qml
        QML_FILES NavButton.qml
        QML_FILES Sidebar.qml
        QML_FILES PlaceholderPage.qml
        QML_FILES DashboardPage.qml
        QML_FILES Theme.qml
        SOURCES alarmrule.h alarmrule.cpp
        SOURCES alarmmanager.h alarmmanager.cpp
        SOURCES alarmmodel.h alarmmodel.cpp
        QML_FILES AlarmsPage.qml
        SOURCES dbworker.h dbworker.cpp
        SOURCES datalogger.h datalogger.cpp
        SOURCES dbpath.h dbpath.cpp
        SOURCES historyreader.h historyreader.cpp
        QML_FILES ChoiceBar.qml
        QML_FILES HistoryPage.qml
        SOURCES appsettings.h appsettings.cpp
        QML_FILES ThresholdRow.qml
        QML_FILES SettingsPage.qml
        QML_FILES NumberField.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appSupervisionDashboard PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appSupervisionDashboard
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appSupervisionDashboard
    PRIVATE Qt6::Quick Qt6::Sql
)
install(TARGETS appSupervisionDashboard
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
