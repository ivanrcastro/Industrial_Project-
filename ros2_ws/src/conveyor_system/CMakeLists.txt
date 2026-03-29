cmake_minimum_required(VERSION 3.8)
project(conveyor_system)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(std_msgs REQUIRED)
find_package(factory_interfaces REQUIRED)

add_executable(conveyor_node src/conveyor_node.cpp)
ament_target_dependencies(conveyor_node 
                          rclcpp 
                          rclcpp_lifecycle 
                          std_msgs 
                          factory_interfaces)

install(TARGETS
  conveyor_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
