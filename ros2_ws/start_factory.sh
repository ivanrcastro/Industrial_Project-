#!/bin/bash

# 1. Iniciar o Broker MQTT (em background)
echo "Starting Mosquitto..."
mosquitto -d

# 2. Source do ROS 2
source /opt/ros/humble/setup.bash
source install/setup.bash

echo "🚀 Starting Industrial Project Fleet in Background..."

# 3. Iniciar a Bridge (Redirecionando logs para ficheiro)
ros2 run amr_controller mqtt_bridge > /tmp/bridge.log 2>&1 &
echo "✅ MQTT Bridge launched (log: /tmp/bridge.log)"

# 4. Iniciar Robôs
ros2 run amr_controller robot_node > /tmp/robot1.log 2>&1 &
ros2 run amr_controller robot_node_2 > /tmp/robot2.log 2>&1 &
echo "✅ AMR Nodes launched"

# 5. Instruções finais
echo "------------------------------------------------"
echo "ROS 2 nodes are running in background."
echo "Check logs at /tmp/robot1.log or /tmp/robot2.log"
echo "NOW: Go to your PC terminal and run: dotnet run"
echo "------------------------------------------------"