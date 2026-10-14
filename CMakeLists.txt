cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 20)

include($ENV{GEODE_SDK}/cmake/GeodeFile.cmake)
project(AutoTrajectoryPlayer)

setup_geode_mod(
  NAME "Auto Trajectory Player"
  ID "puttipongnark.auto_trajectory_player"
)

add_subdirectory(src)
