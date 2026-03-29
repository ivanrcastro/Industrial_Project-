#include <chrono>
#include <string>
#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "factory_interfaces/msg/robot_state.hpp"
#include "std_msgs/msg/string.hpp" // Para enviar o comando ao robô
#include "mqtt/async_client.h"

using namespace std::chrono_literals;

class MqttBridge : public rclcpp::Node {
public:
    MqttBridge() : Node("mqtt_bridge"), 
                   mqtt_client_("tcp://mqtt_broker:1883", "ros2_bridge_client") {
        
        auto connOpts = mqtt::connect_options_builder()
            .keep_alive_interval(20s)
            .clean_session(true)
            .automatic_reconnect(true)
            .finalize();

        try {
            RCLCPP_INFO(this->get_logger(), "A tentar ligar ao Broker MQTT...");
            
            // 1. Configurar o callback ANTES de conectar
            mqtt_client_.set_message_callback([this](mqtt::const_message_ptr msg) {
                this->mqtt_to_ros_callback(msg);
            });

            // 2. Conectar
            mqtt_client_.connect(connOpts)->wait();
            
            // 3. Subscrever DEPOIS de conectar
            mqtt_client_.subscribe("factory/fleet/control", 1);

            RCLCPP_INFO(this->get_logger(), "✅ Bridge Bidirecional Ativo e Subscrito!");
        }
        catch (const mqtt::exception& exc) {
            RCLCPP_FATAL(this->get_logger(), "❌ Erro MQTT: %s", exc.what());
            throw std::runtime_error("Falha na conexão");
        }

        // Publisher para enviar comandos para o robô
        cmd_publisher_ = this->create_publisher<std_msgs::msg::String>("robot_commands", 10);

        // Subscrição para receber telemetria do robô
        subscription_ = this->create_subscription<factory_interfaces::msg::RobotState>(
            "robot_state", 10, std::bind(&MqttBridge::callback, this, std::placeholders::_1));
    }

private:
    // 1. ROS2 -> MQTT (Telemetria)
    void callback(const factory_interfaces::msg::RobotState::SharedPtr msg) {
    std::stringstream ss;
    ss << "{"
       << "\"id\":\"" << msg->robot_id << "\", "
       << "\"battery\":" << std::fixed << std::setprecision(1) << msg->battery << ", "
       << "\"status\":\"" << msg->status << "\", "
       << "\"x\":" << std::fixed << std::setprecision(2) << msg->pose.position.x << ", "
       << "\"y\":" << std::fixed << std::setprecision(2) << msg->pose.position.y
       << "}";
    
    mqtt_client_.publish("factory/fleet/status", ss.str(), 1, false);
}

    // 2. MQTT -> ROS2 (Comandos de Controlo) - O QUE FALTA!
    void mqtt_to_ros_callback(mqtt::const_message_ptr msg) {
        std::string payload = msg->to_string();
        RCLCPP_WARN(this->get_logger(), "📩 Comando recebido via MQTT: [%s]", payload.c_str());

        auto ros_msg = std_msgs::msg::String();
        ros_msg.data = payload;
        
        // Envia para o robô via ROS2
        cmd_publisher_->publish(ros_msg);
    }

    mqtt::async_client mqtt_client_;
    rclcpp::Subscription<factory_interfaces::msg::RobotState>::SharedPtr subscription_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr cmd_publisher_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MqttBridge>());
    rclcpp::shutdown();
    return 0;
}