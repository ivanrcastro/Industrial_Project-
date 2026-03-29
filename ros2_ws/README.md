🏭 Factory Fleet Manager (ROS 2 + C#)

A distributed industrial simulation featuring a fleet of Autonomous Mobile Robots (AMR) controlled individually via a C# Dashboard. The system uses MQTT as a communication bridge and ROS 2 for robot logic, telemetry, and 3D visualization.
🏗️ System Architecture

The project is divided into three main layers:

    Control Layer (C#): A high-level dashboard for monitoring battery, status, and position.

    Bridge Layer (MQTT/ROS 2): A C++ node that translates MQTT JSON payloads into ROS 2 messages.

    Simulation Layer (ROS 2): Individual robot nodes (AMR_01, AMR_02) handling kinematics and TF2 transforms.

🚀 Execution Guide

To run the complete system, open 5 terminals in the following order (ensure you have sourced your ROS 2 workspace: source install/setup.bash):
Order	Component	Terminal / Command	Purpose
1	MQTT Broker	mosquitto -v	Handles messaging between C# and ROS 2.
2	MQTT Bridge	ros2 run amr_controller mqtt_bridge	Bridges the MQTT/ROS 2 communication gap.
3	AMR Nodes	

ros2 run amr_controller robot_node

ros2 run amr_controller robot_node_2
	Simulates robot logic, battery, and movement.
4	Visualization	rviz2	Setup: Set Fixed Frame to world and add TF/Marker.
5	Dashboard	dotnet run (inside /FleetManager)	Real-time monitoring and individual control.

🎮 Dashboard Commands
Key	Target	Action
1 / 2	AMR_01 / 02	Emergency Stop (Stop selected robot)
Q / W	AMR_01 / 02	Reset (Resume navigation)
A / S	AMR_01 / 02	Charge (Send selected robot to charge)
X	All	Global Stop (Freeze the entire fleet)
🛠️ Project Structure

    amr_controller/: ROS 2 C++ package containing robot nodes and the MQTT bridge.

    FleetManager/: C# Console Application built with Spectre.Console and MQTTnet.

    factory_interfaces/: Custom ROS 2 message definitions for Robot State (Battery, Pose, Status).

📈 Roadmap

    [ ] Integrate URDF models for realistic visualization (KUKA LBR iiwa).

    [ ] Add Environmental Markers (Walls, Charging Stations) in RViz.

    [ ] Implement basic Obstacle Avoidance between robots.

💡 Troubleshooting

    Check MQTT Traffic: Use mosquitto_sub -t "factory/fleet/status" to see raw JSON.

    Check ROS TFs: Run ros2 run tf2_tools view_frames to verify coordinate trees.

    Clean Build: If errors occur, run rm -rf build/ install/ log/ before rebuilding.