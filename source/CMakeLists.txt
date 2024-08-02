if (DEFINED ENV_WX_CONFIG)
    set (ENV{WX_CONFIG} ${ENV_WX_CONFIG})
endif()

set(wxWidgets_USE_STATIC 1)

find_package(wxWidgets REQUIRED)

set(SRCS main.cpp 
    drawingcanvas.cpp chartcontrol.cpp
    Kicad80Symbol.cpp 
    KicadSymbol.cpp KicadFootprint.cpp 
    SymboltoJson.cpp analysisLCSymbol.cpp
    convertAltiumFp.cpp 
    LCSYMSort.cpp SortLCSymbolInfo.cpp
    )

include(${wxWidgets_USE_FILE})

if (APPLE)
    add_executable(main MACOSX_BUNDLE ${SRCS} )
    set_target_properties(main PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
else()
    add_executable(main WIN32 ${SRCS} main.exe.manifest)
endif()

target_link_libraries(main PRIVATE 
                ${wxWidgets_LIBRARIES}
                myproject::sexpr_parser
                symbolKiCad8
                )

