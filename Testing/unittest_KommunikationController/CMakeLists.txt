set(CMAKE_AUTOMOC ON)

qt_add_executable(unittest_SystemController
    tst_kommunikationController.cpp

    ../app/src/logic/kommunikationController.cpp
    ../app/src/logic/kommunikationsModul.cpp
    # ../app/src/logic/antPlusModul.cpp
    # ../app/src/logic/bluetoothModul.cpp
    # ../app/src/logic/wlanModul.cpp

    ../app/include/logic/kommunikationController.h
    ../app/include/logic/kommunikationsModul.h

    # ../app/include/logic/wlanModul.h
    # ../app/include/logic/bluetoothModul.h
    # ../app/include/logic/antPlusModul.h

)


target_include_directories(unittest_SystemController PRIVATE
    ../app/include
)

target_link_libraries(unittest_SystemController PRIVATE
    Qt6::Test
)

add_test(NAME unittest_SystemController COMMAND unittest_SystemController)