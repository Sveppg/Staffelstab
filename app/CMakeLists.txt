set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# set_property(TARGET se-2-dragon-slayer-2 PROPERTY AUTOMOC ON)

qt_add_executable(se_2_dragon_slayer_2
    src/main.cpp
    # src/appconfig.cpp
    src/mainwindow.cpp
    # src/flightsystem.cpp

    # Logic Classes
    include/logic/startstopbutton.h
    src/logic/startstopbutton.cpp

    include/logic/menuebutton.h
    src/logic/menuebutton.cpp

    include/logic/antPlusModul.h
    src/logic/antPlusModul.cpp

    include/logic/bluetoothModul.h
    src/logic/bluetoothModul.cpp

    include/logic/gpsModul.h
    src/logic/gpsModul.cpp

    include/logic/gyroskopsensor.h
    src/logic/gyroskopsensor.cpp

    include/logic/trittfrequenzsensor.h
    src/logic/trittfrequenzsensor.cpp

    include/logic/beschleunigungssensor.h
    src/logic/beschleunigungssensor.cpp

    include/logic/hardwaredaten.h

    include/logic/hardwaresteuerung.h
    src/logic/hardwaresteuerung.cpp

    include/logic/vibrationsmotor.h
    src/logic/vibrationsmotor.cpp

    include/logic/kommunikationController.h
    src/logic/kommunikationController.cpp

    include/logic/kommunikationsModul.h
    src/logic/kommunikationsModul.cpp

    include/logic/nearFieldModul.h
    src/logic/nearFieldModul.cpp

    include/logic/ortung.h
    src/logic/ortung.cpp

    include/logic/wlanModul.h
    src/logic/wlanModul.cpp

    include/logic/wlanNetzwerk.h
    src/logic/wlanNetzwerk.cpp

    include/logic/ledRing.h
    src/logic/ledRing.cpp

    include/logic/display.h
    src/logic/display.cpp

    include/logic/ladehub.h
    src/logic/ladehub.cpp

    include/logic/laufkoordination.h
    src/logic/laufkoordination.cpp

    include/logic/herzfrequenzsensor.h
    src/logic/herzfrequenzsensor.cpp


    # Simulation Classes
    include/simulation/physicsengine.h
    include/simulation/clock.h
    src/simulation/physicsengine.cpp
    src/simulation/clock.cpp


    include/laufsystem.h
    src/laufsystem.cpp

    # include/appconfig.h
    include/mainwindow.h
    # include/flightsystem.h
    mainwindow.ui
)

qt_add_resources(se_2_dragon_slayer_2 "data"
    PREFIX "/"
    BASE "data"
    FILES
)

# Directories searched when including header files —
# no need for explicit paths like #include "../../include/foo.h" in source files
target_include_directories(se_2_dragon_slayer_2 PRIVATE include)
# target_include_directories(se_2_dragon_slayer_2 PRIVATE include/simulation)
# target_include_directories(se_2_dragon_slayer_2 PRIVATE include/logic)

# Tell AUTOUIC where to find .ui files (they live in app/, not app/src/),
# and always place the executable in <build-dir>/dist regardless of platform or kit
set_target_properties(se_2_dragon_slayer_2 PROPERTIES
    AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist
)


target_link_libraries(se_2_dragon_slayer_2 PRIVATE Qt6::Core Qt6::Widgets)
