qt_add_executable(unittest_Ortung
    tst_ortung.cpp
    ../app/src/logic/gpsModul.cpp
    ../app/src/logic/nearFieldModul.cpp
    ../app/src/logic/ortung.cpp
    ../app/include/logic/gpsModul.h
    ../app/include/logic/nearFieldModul.h
    ../app/include/logic/ortung.h
)

target_include_directories(unittest_Ortung PRIVATE ../app/include)

target_link_libraries(unittest_Ortung PRIVATE Qt6::Test)

add_test(NAME unittest_Ortung COMMAND unittest_Ortung)
