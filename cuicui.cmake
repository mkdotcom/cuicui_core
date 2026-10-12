# cuicui core sources for CMake-based builds (Android).
# Usage in your CMakeLists.txt:
#     include(<path to>/cuicui_core/cuicui.cmake)
#     cuicui_add_sources(<your target>)
# The parent folder of cuicui_core/ must be in the include path, so that
# `#include "cuicui_core/cuicui.h"` works.

set(CUICUI_CORE_DIR ${CMAKE_CURRENT_LIST_DIR})

set(CUICUI_CORE_SOURCES
    ${CUICUI_CORE_DIR}/cuicui.cpp
    ${CUICUI_CORE_DIR}/central.cpp
    ${CUICUI_CORE_DIR}/easing.cpp
    ${CUICUI_CORE_DIR}/local_storage.cpp
    ${CUICUI_CORE_DIR}/timer.cpp
    ${CUICUI_CORE_DIR}/web_request.cpp
    ${CUICUI_CORE_DIR}/wBase.cpp
    ${CUICUI_CORE_DIR}/wImage.cpp
    ${CUICUI_CORE_DIR}/wScrollView.cpp
    ${CUICUI_CORE_DIR}/wText.cpp
    ${CUICUI_CORE_DIR}/wTextInput.cpp
    ${CUICUI_CORE_DIR}/nws/wCalendar.cpp
    ${CUICUI_CORE_DIR}/nws/wTimePicker.cpp
)

if(ANDROID)
    list(APPEND CUICUI_CORE_SOURCES ${CUICUI_CORE_DIR}/android/native-lib.cpp)
endif()

function(cuicui_add_sources target)
    target_sources(${target} PRIVATE ${CUICUI_CORE_SOURCES})
    target_include_directories(${target} PRIVATE ${CUICUI_CORE_DIR}/..)
    if(ANDROID)
        find_library(CUICUI_LOG_LIB log)
        target_link_libraries(${target} ${CUICUI_LOG_LIB} android)
    endif()
endfunction()
