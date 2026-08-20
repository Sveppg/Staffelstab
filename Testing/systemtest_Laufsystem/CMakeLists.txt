qt_add_executable(SystemtestLaufsystem
    tst_laufsystem.cpp

    ../app/src/laufsystem.cpp

    ../app/src/simulation/clock.cpp
    ../app/src/simulation/physicsengine.cpp

    ../app/src/logic/startstopbutton.cpp
    ../app/src/logic/laufkoordination.cpp
    ../app/src/logic/ortung.cpp
    ../app/src/logic/gpsModul.cpp
    ../app/src/logic/nearFieldModul.cpp
    ../app/src/logic/trittfrequenzsensor.cpp
    ../app/src/logic/beschleunigungssensor.cpp
    ../app/src/logic/herzfrequenzsensor.cpp
    ../app/src/logic/gyroskopsensor.cpp

    ../app/include/laufsystem.h

    ../app/include/simulation/clock.h
    ../app/include/simulation/physicsengine.h

    ../app/include/logic/startstopbutton.h
    ../app/include/logic/laufkoordination.h
    ../app/include/logic/ortung.h
    ../app/include/logic/gpsModul.h
    ../app/include/logic/nearFieldModul.h
    ../app/include/logic/trittfrequenzsensor.h
    ../app/include/logic/beschleunigungssensor.h
    ../app/include/logic/herzfrequenzsensor.h
    ../app/include/logic/gyroskopsensor.h
)

target_include_directories(SystemtestLaufsystem PRIVATE ../app/include)

target_link_libraries(SystemtestLaufsystem PRIVATE Qt6::Core Qt6::Test)

add_test(NAME SystemtestLaufsystem COMMAND SystemtestLaufsystem)
