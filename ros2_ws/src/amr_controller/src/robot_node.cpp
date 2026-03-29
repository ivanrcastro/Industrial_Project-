#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "factory_interfaces/msg/robot_state.hpp"
#include "std_msgs/msg/string.hpp"

// Bibliotecas para Visualização (TF2)
#include "tf2_ros/transform_broadcaster.h"
#include "geometry_msgs/msg/transform_stamped.hpp"

using namespace std::chrono_literals;

class RobotController : public rclcpp::Node {
public:
    RobotController() : Node("amr_01"), current_state_(NAVIGATING), battery_level_(100.0) {
        // Publisher de Telemetria
        publisher_ = this->create_publisher<factory_interfaces::msg::RobotState>("robot_state", 10);
        
        // Subscriber de Comandos
        command_sub_ = this->create_subscription<std_msgs::msg::String>(
            "robot_commands", 10, std::bind(&RobotController::command_callback, this, std::placeholders::_1));

        // --- INICIALIZAÇÃO DO TF2 ---
        tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

        timer_ = this->create_wall_timer(500ms, std::bind(&RobotController::publish_state, this));
        
        RCLCPP_INFO(this->get_logger(), "AMR_01 iniciado com suporte RViz.");
    }

private:
    void command_callback(const std_msgs::msg::String::SharedPtr msg) {
        std::string data = msg->data;
        bool for_me = (data.find("\"target\":\"AMR_01\"") != std::string::npos) || 
                      (data.find("\"target\":\"ALL\"") != std::string::npos);

        if (!for_me) return;

        if (data.find("\"cmd\":\"EMERGENCY_STOP\"") != std::string::npos) {
            current_state_ = EMERGENCY_STOP;
        } else if (data.find("\"cmd\":\"RESET\"") != std::string::npos) {
            current_state_ = NAVIGATING;
        } else if (data.find("\"cmd\":\"CHARGE\"") != std::string::npos) {
            current_state_ = CHARGING;
        }
    }

    void publish_state() {
        auto message = factory_interfaces::msg::RobotState();
        message.robot_id = "AMR_01";
        
        // Lógica de Movimento
        switch (current_state_) {
            case NAVIGATING:
                message.status = "NAVIGATING";
                if (battery_level_ > 0) battery_level_ -= 0.01;
                x_ += 0.1 * cos(theta_);
                y_ += 0.1 * sin(theta_);
                theta_ += 0.05;
                break;
            case EMERGENCY_STOP:
                message.status = "EMERGENCY_STOP";
                break;
            case CHARGING:
                message.status = "CHARGING";
                if (battery_level_ < 100.0) battery_level_ += 0.05;
                break;
        }

        message.battery = battery_level_;
        message.pose.position.x = x_;
        message.pose.position.y = y_;
        publisher_->publish(message);

        // --- LÓGICA DO TF2 (CORRIGIDA) ---
        geometry_msgs::msg::TransformStamped t;
        t.header.stamp = this->get_clock()->now();
        t.header.frame_id = "world";      // Frame pai
        t.child_frame_id = "amr_01";      // Frame filho (o robô)
        t.transform.translation.x = x_;
        t.transform.translation.y = y_;
        t.transform.translation.z = 0.0;
        t.transform.rotation.z = sin(theta_ / 2.0);
        t.transform.rotation.w = cos(theta_ / 2.0);

        tf_broadcaster_->sendTransform(t);
    }
    
    enum State { NAVIGATING, EMERGENCY_STOP, CHARGING };
    State current_state_;
    double battery_level_, x_ = 0.0, y_ = 0.0, theta_ = 0.0;
    
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<factory_interfaces::msg::RobotState>::SharedPtr publisher_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr command_sub_;
    
    // --- DECLARAÇÃO DO BROADCASTER ---
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<RobotController>());
    rclcpp::shutdown();
    return 0;
}