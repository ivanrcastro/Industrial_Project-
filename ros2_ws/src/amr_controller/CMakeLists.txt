cmake_minimum_required(VERSION 3.8)
project(amr_controller)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 1. Encontrar dependências
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(factory_interfaces REQUIRED)
find_package(PahoMqttCpp REQUIRED) # Biblioteca de sistema
find_package(std_msgs REQUIRED) #
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(visualization_msgs REQUIRED)

# 2. Executável do Robô (Apenas ROS)
add_executable(robot_node src/robot_node.cpp)
ament_target_dependencies(robot_node 
  rclcpp 
  factory_interfaces 
  std_msgs 
  tf2 
  tf2_ros 
  geometry_msgs
  visualization_msgs
)

# 2. Executável do Robô 2 (GARANTE QUE ESTÁ ASSIM)
add_executable(robot_node_2 src/robot_node_2.cpp)
ament_target_dependencies(robot_node_2 
  rclcpp 
  factory_interfaces 
  std_msgs 
  tf2 
  tf2_ros 
  geometry_msgs
  visualization_msgs
)
# 3. Executável do Bridge (ROS + MQTT)
add_executable(mqtt_bridge src/mqtt_bridge.cpp)

ament_target_dependencies(mqtt_bridge rclcpp factory_interfaces std_msgs tf2 tf2_ros geometry_msgs
)

# Ligamos a biblioteca externa manualmente (O TRUQUE É ESTE)
target_link_libraries(mqtt_bridge PahoMqttCpp::paho-mqttpp3)

# 4. Instalação
install(TARGETS 
  robot_node 
  robot_node_2
  mqtt_bridge 
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()