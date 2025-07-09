cmake_minimum_required(VERSION 3.5)

# 定义语言
# 在 project() 之后添加 C 语言支持
project(buju2 VERSION 0.1 LANGUAGES C CXX)

# 定义一个库目标，将指定的源文件编译成一个库，这里是静态库
# 确保 kiss_fft.c 以 C 模式编译
add_library(kissfft STATIC
    kissfft/kiss_fft.c
)
set_source_files_properties(kissfft/kiss_fft.c PROPERTIES LANGUAGE C)  # 强制使用 C 编译器

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#查找Qt组件
find_package(Qt6 REQUIRED COMPONENTS Sql)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Charts)
find_package(Qt6 REQUIRED COMPONENTS SerialPort)

#定义PROJECT_SOURCES变量 里面存放项目的主要源文件列表
set(PROJECT_SOURCES
        main.cpp
        widget.cpp
        widget.h
        dcm1.h
        dcm1.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(buju2
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        dcm1.h dcm1.cpp
        show_sql.h show_sql.cpp
        itemform.h itemform.cpp
        usarthelper.h usarthelper.cpp
        database_manager.h
        foc1.h
        foc1.cpp
        diagnosiswindow.h
        diagnosiswindow.cpp
        pic.qrc
        dcm2.h
        dcm2.cpp
        dcm3.h
        dcm3.cpp
        dcm4.h
        dcm4.cpp
        dcm5.h dcm5.cpp
        dcm6.h dcm6.cpp
        dcm7.h dcm7.cpp
        dcm8.h dcm8.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET buju2 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(buju2 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(buju2
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(buju2 PRIVATE kissfft)
target_link_libraries(buju2 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(buju2 PRIVATE Qt6::Charts)
target_link_libraries(buju2 PRIVATE Qt6::Sql)
target_include_directories(buju2 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})  # 添加项目根目录到头文件搜索路径
target_include_directories(kissfft PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/kissfft)
target_link_libraries(buju2 PRIVATE kissfft)
target_link_libraries(buju2 PRIVATE Qt6::SerialPort)


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.buju2)
endif()
set_target_properties(buju2 PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS buju2
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(buju2)

endif()

# cmake_minimum_required(VERSION 3.5)

# project(buju1 VERSION 0.1 LANGUAGES C CXX)

# # 设置 Qt6 路径（核心组件在默认路径，Charts/SerialPort 在自定义路径）
# set(Qt6_DIR "/usr/lib/aarch64-linux-gnu/cmake/Qt6")
# set(Qt6Charts_DIR "/usr/local/lib/aarch64-linux-gnu/cmake/Qt6Charts")
# set(Qt6SerialPort_DIR "/usr/local/lib/aarch64-linux-gnu/cmake/Qt6SerialPort")

# # 查找 Qt6 组件（必须首先调用 find_package 才能获取 QT_VERSION_MAJOR）
# find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Charts SerialPort)

# # 打印调试信息
# message(STATUS "Qt6 version: ${Qt6_VERSION}")
# message(STATUS "Qt6 Core found: ${Qt6Core_FOUND}")
# message(STATUS "Qt6 Charts found: ${Qt6Charts_FOUND}")

# # 定义静态库 kissfft
# add_library(kissfft STATIC kissfft/kiss_fft.c)
# set_source_files_properties(kissfft/kiss_fft.c PROPERTIES LANGUAGE C)

# # 启用 Qt 自动处理
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # 定义项目源文件
# set(PROJECT_SOURCES
#     main.cpp
#     widget.cpp
#     widget.h
#     dcm1.h
#     dcm1.cpp
#     show_sql.h show_sql.cpp
#     itemform.h itemform.cpp
#     usarthelper.h usarthelper.cpp
#     database_manager.h
#     dcm2.h dcm2.cpp
#     dcm3.h dcm3.cpp
#     dcm4.h dcm4.cpp
#     dcm5.h dcm5.cpp
#     dcm6.h dcm6.cpp
#     dcm7.h dcm7.cpp
#     dcm8.h dcm8.cpp
# )

# # 添加可执行文件
# qt_add_executable(buju1 MANUAL_FINALIZATION ${PROJECT_SOURCES})

# # 链接库
# target_link_libraries(buju1 PRIVATE
#     kissfft
#     Qt6::Core
#     Qt6::Widgets
#     Qt6::Sql
#     Qt6::Charts
#     Qt6::SerialPort
# )

# # 包含目录
# target_include_directories(buju1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# target_include_directories(kissfft PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/kissfft)

# # 修复版本检查：使用 Qt6_VERSION 而不是 QT_VERSION
# if(${Qt6_VERSION} VERSION_LESS 6.1.0)
#   set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.buju1)
# endif()

# set_target_properties(buju1 PROPERTIES
#     ${BUNDLE_ID_OPTION}
#     MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
#     MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
#     MACOSX_BUNDLE TRUE
#     WIN32_EXECUTABLE TRUE
# )

# # 安装配置
# include(GNUInstallDirs)
# install(TARGETS buju1
#     BUNDLE DESTINATION .
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )

# # Qt6 最终化处理
# qt_finalize_executable(buju1)
