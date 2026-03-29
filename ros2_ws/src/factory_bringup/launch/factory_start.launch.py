from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        # Iniciar o Gestor da Passadeira (C++)
        Node(
            package='conveyor_system',
            executable='conveyor_node',
            name='conveyor_manager',
            output='screen'
        ),
        # Aqui mais tarde adicionaremos o amr_controller e o bridge MQTT
    ])