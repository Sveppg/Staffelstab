qt_add_executable(unittest_Hardwaresteuerung
    utst_hardwaresteuerung.cpp

    ../app/src/logic/hardwaresteuerung.cpp
    ../app/src/logic/startstopbutton.cpp
    ../app/src/logic/menuebutton.cpp
    ../app/src/logic/vibrationsmotor.cpp
    ../app/src/logic/display.cpp
    ../app/src/logic/ledRing.cpp
    ../app/src/logic/ladehub.cpp

    ../app/include/logic/hardwaresteuerung.h
    ../app/include/logic/startstopbutton.h
    ../app/include/logic/menuebutton.h
    ../app/include/logic/vibrationsmotor.h
    ../app/include/logic/display.h
    ../app/include/logic/ledRing.h
    ../app/include/logic/ladehub.h
    ../app/include/logic/hardwaredaten.h
)

target_include_directories(unittest_Hardwaresteuerung PRIVATE ../app/include)

target_link_libraries(unittest_Hardwaresteuerung PRIVATE Qt6::Test)

add_test(NAME unittest_Hardwaresteuerung COMMAND unittest_Hardwaresteuerung)

