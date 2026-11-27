cmake_minimum_required(VERSION 3.20)
project(M_MEdit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动收集 src 目录下的源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 创建 Windows 可执行程序 (WIN32 表示 GUI 入口 WinMain，不弹黑框控制台)
add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

# 链接选项：支持 Unicode WinMain 入口
if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -municode)
endif()

# 包含头文件目录
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/packages/WebView2/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# 链接 WebView2Loader 动态库与 Windows 系统核心库
target_link_libraries(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/packages/WebView2/lib/x64/WebView2Loader.dll"
    ole32
    oleaut32
    user32
    gdi32
    shell32
    comdlg32
    shlwapi
    uuid
)

# 编译后自动拷贝 WebView2Loader.dll 和 assets 静态资源到输出目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/packages/WebView2/lib/x64/WebView2Loader.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
)
