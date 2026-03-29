# 🏭 Factory Fleet Manager (ROS 2 + C#)

A distributed industrial simulation featuring a fleet of Autonomous
Mobile Robots (AMR). This project demonstrates real-time monitoring and
control using MQTT as a high-speed communication bridge between ROS 2
(C++) and a professional C# Dashboard.

## 🏗️ System Architecture

The project is built on three decoupled layers:

-   **Control Layer (C#):** A high-level dashboard for real-time
    monitoring and command issuing.
-   **Bridge Layer (MQTT/ROS 2):** A C++ node that bi-directionally
    translates MQTT JSON payloads into ROS 2 messages.
-   **Simulation Layer (ROS 2):** Individual robot nodes (AMR_01,
    AMR_02) handling kinematics and TF2 transforms.

## 🚀 Execution Guide (Simplified)

Instead of manual terminal management, use the provided automation
script.

### 1. Launch ROS 2 Stack (Docker/Linux)

In your main terminal, run:

``` bash
chmod +x start_factory.sh
./start_factory.sh
```

This starts the Mosquitto Broker, the MQTT Bridge, and all Robot Nodes
in the background.

### 2. Launch Control Dashboard (Host PC)

In a new terminal on your computer, run:

``` bash
cd FleetManager
dotnet run
```

### 3. Visualization

Open RViz2 to see the robots in 3D space:

-   Set Fixed Frame to `world`.
-   Add TF and Marker displays to visualize the fleet.

## 🎮 Dashboard Commands

  Key     Target        Action
  ------- ------------- -----------------------------------------
  1 / 2   AMR_01 / 02   Emergency Stop (Immediate freeze)
  Q / W   AMR_01 / 02   Reset (Resume normal navigation)
  A / S   AMR_01 / 02   Charge (Send robot to docking station)
  X       All Robots    Global Stop (Fleet-wide emergency halt)

## 🛠️ Maintenance & Debugging

-   **Stop All Background Processes:**\
    If you need to restart cleanly, run:\
    `pkill -f amr_controller && pkill mosquitto`

-   **Check Logs:**\
    The launch script redirects output to:\
    `/tmp/bridge.log`, `/tmp/robot1.log`, `/tmp/robot2.log`.

-   **Monitor MQTT:**\
    Use:\
    `mosquitto_sub -t "factory/fleet/status"`\
    to see raw telemetry.

## 📈 Roadmap

-   [ ] KUKA LBR iiwa Integration: Replace basic markers with URDF
    models.
-   [ ] Environmental Mapping: Add static markers (walls, zones) in
    RViz.
-   [ ] Collision Avoidance: Implement basic proximity logic between
    AMRs.