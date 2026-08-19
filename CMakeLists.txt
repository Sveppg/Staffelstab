cmake_minimum_required(VERSION 3.19)
project(se2-Dragon-Slayer-2 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Test)

# Activates AUTOMOC, AUTOUIC, AUTORCC and sets sensible Qt defaults
qt_standard_project_setup()

enable_testing()

# Using subdirs to separate the application and unit tests.
# Qt recommends putting each unit test class in its own target.
add_subdirectory(app)
add_subdirectory(unittest_KommunikationController)
add_subdirectory(unittest_Ortung)
add_subdirectory(unittest_Laufkoordination)
add_subdirectory(integrationtest_Hardwaresteuerung)
add_subdirectory(unittest_Hardwaresteuerung)
add_subdirectory(systemtest_Laufsystem)
