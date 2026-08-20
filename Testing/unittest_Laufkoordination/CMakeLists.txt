set(CMAKE_AUTOMOC ON)
qt_add_executable(unittest_Laufkoordination
    tst_laufkoordination.cpp
    ../app/src/logic/laufkoordination.cpp
    ../app/src/logic/ortung.cpp
    ../app/src/logic/gpsModul.cpp
    ../app/src/logic/nearFieldModul.cpp
    ../app/src/logic/trittfrequenzsensor.cpp
    ../app/src/logic/beschleunigungssensor.cpp
    ../app/src/logic/herzfrequenzsensor.cpp
    ../app/src/logic/gyroskopsensor.cpp

    ../app/include/logic/laufkoordination.h
    ../app/include/logic/ortung.h
    ../app/include/logic/gpsModul.h
    ../app/include/logic/nearFieldModul.h
    ../app/include/logic/trittfrequenzsensor.h
    ../app/include/logic/beschleunigungssensor.h
    ../app/include/logic/herzfrequenzsensor.h
    ../app/include/logic/gyroskopsensor.h
)

target_include_directories(unittest_Laufkoordination PRIVATE ../app/include)

target_link_libraries(unittest_Laufkoordination PRIVATE Qt6::Test)

add_test(NAME unittest_Laufkoordination COMMAND unittest_Laufkoordination)
