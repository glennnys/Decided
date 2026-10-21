cmake_minimum_required(VERSION 3.19)
project(Decided LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 COMPONENTS Sql REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



qt_standard_project_setup()

qt_add_executable(Decided
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    mealpage.h mealpage.cpp mealpage.ui
    activitypage.h activitypage.cpp activitypage.ui

    genactivity.h genactivity.cpp genactivity.ui
    mngitems.h mngitems.cpp mngitems.ui
    mngactivity.h mngactivity.cpp mngactivity.ui
    mainsettings.h mainsettings.cpp mainsettings.ui

    utils.h utils.cpp
    mngtags.h mngtags.cpp mngtags.ui
    tag.h tag.cpp tag.ui
)

target_link_libraries(Decided
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::Sql
)

include(GNUInstallDirs)

install(TARGETS Decided
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Decided
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
