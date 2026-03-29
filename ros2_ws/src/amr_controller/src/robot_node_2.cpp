#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "factory_interfaces/msg/robot_state.hpp"
#include "std_msgs/msg/string.hpp"

// Bibliotecas para Visualização (TF2 e Markers)
#include "tf2_ros/transform_broadcaster.h"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "visualization_msgs/msg/marker.hpp"

using namespace std::chrono_literals;

class RobotController : public rclcpp::Node {
public:
    RobotController() : Node("amr_02"), current_state_(NAVIGATING), battery_level_(100.0) {
        
        // 1. Publishers
        publisher_ = this->create_publisher<factory_interfaces::msg::RobotState>("robot_state", 10);
        marker_pub_ = this->create_publisher<visualization_msgs::msg::Marker>("robot_marker", 10);
        
        // 2. Subscriber de Comandos
        command_sub_ = this->create_subscription<std_msgs::msg::String>(
            "robot_commands", 10, std::bind(&RobotController::command_callback, this, std::placeholders::_1));

        // 3. Broadcaster de TF (Transformadas)
        tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

        // 4. Timer de atualização (500ms)
        timer_ = this->create_wall_timer(500ms, std::bind(&RobotController::publish_state, this));
        
        RCLCPP_INFO(this->get_logger(), "AMR_02 (Kuka iiwa Style) ativo e visível no RViz.");
    }

private:
    void command_callback(const std_msgs::msg::String::SharedPtr msg) {
        std::string data = msg->data;
        bool for_me = (data.find("\"target\":\"AMR_02\"") != std::string::npos) || 
                      (data.find("\"target\":\"ALL\"") != std::string::npos);

        if (for_me) {
            if (data.find("\"cmd\":\"EMERGENCY_STOP\"") != std::string::npos) {
                current_state_ = EMERGENCY_STOP;
                RCLCPP_ERROR(this->get_logger(), "🛑 AMR_02: PARADO!");
            } 
            else if (data.find("\"cmd\":\"RESET\"") != std::string::npos) {
                current_state_ = NAVIGATING;
                RCLCPP_INFO(this->get_logger(), "✅ AMR_02: RETOMADO!");
            } 
            else if (data.find("\"cmd\":\"CHARGE\"") != std::string::npos) {
                current_state_ = CHARGING;
            }
        }
    }

    void publish_state() {
        auto message = factory_interfaces::msg::RobotState();
        message.robot_id = "AMR_02"; 
        
        // Lógica de Movimento Circular
        switch (current_state_) {
            case NAVIGATING:
                message.status = "NAVIGATING";
                if (battery_level_ > 0) battery_level_ -= 0.015; 
                x_ -= 0.08 * cos(theta_); 
                y_ += 0.08 * sin(theta_);
                theta_ += 0.03;
                break;
            case EMERGENCY_STOP: 
                message.status = "EMERGENCY_STOP"; 
                break;
            case CHARGING:
                message.status = "CHARGING";
                if (battery_level_ < 100.0) battery_level_ += 0.08; 
                break;
        }

        message.battery = battery_level_;
        message.pose.position.x = x_;
        message.pose.position.y = y_;
        publisher_->publish(message);

        // --- PUBLICAR TF (Para o RViz saber onde o robô está) ---
        geometry_msgs::msg::TransformStamped t;
        t.header.stamp = this->get_clock()->now();
        t.header.frame_id = "world";
        t.child_frame_id = "amr_02";
        t.transform.translation.x = x_;
        t.transform.translation.y = y_;
        t.transform.translation.z = 0.0;
        t.transform.rotation.z = sin(theta_ / 2.0);
        t.transform.rotation.w = cos(theta_ / 2.0);
        tf_broadcaster_->sendTransform(t);

        // --- PUBLICAR MARKER (O corpo visual do robô) ---
        auto marker = visualization_msgs::msg::Marker();
        marker.header.frame_id = "amr_02"; // Fixado ao frame do robô
        marker.header.stamp = this->get_clock()->now();
        marker.ns = "amr_visual";
        marker.id = 2;
        marker.type = visualization_msgs::msg::Marker::CYLINDER; 
        marker.action = visualization_msgs::msg::Marker::ADD;
        
        // Dimensões (parecido com uma base LBR iiwa)
        marker.scale.x = 0.4; marker.scale.y = 0.4; marker.scale.z = 0.3;
        
        // Cor: Laranja Kuka (R:1.0, G:0.5, B:0.0)
        marker.color.a = 1.0; marker.color.r = 1.0; marker.color.g = 0.5; marker.color.b = 0.0;
        
        marker_pub_->publish(marker);
    }
    
    enum State { NAVIGATING, EMERGENCY_STOP, CHARGING };
    State current_state_;
    double battery_level_, x_ = 5.0, y_ = 5.0, theta_ = 0.0;
    
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<factory_interfaces::msg::RobotState>::SharedPtr publisher_;
    rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr marker_pub_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr command_sub_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<RobotController>());
    rclcpp::shutdown();
    return 0;
}