#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "factory_interfaces/msg/robot_state.hpp" 

class ConveyorNode : public rclcpp::Node {
public:
    ConveyorNode() : Node("conveyor_manager") {
        // Subscreve ao estado do robô para saber quando ele chega
        // ... dentro da classe ConveyorNode
    subscription_ = this->create_subscription<factory_interfaces::msg::RobotState>(
        "robot_state", 10, std::bind(&ConveyorNode::topic_callback, this, std::placeholders::_1));
    
    RCLCPP_INFO(this->get_logger(), "Sistema de Conveyor Inicializado e a aguardar robôs...");
    }

private:
    void topic_callback(const factory_interfaces::msg::RobotState::SharedPtr msg) {
        if (msg->status == "ARRIVED") {
            RCLCPP_INFO(this->get_logger(), "Robô %s chegou. A ligar passadeira!", msg->robot_id.c_str());
        }
    }
    rclcpp::Subscription<factory_interfaces::msg::RobotState>::SharedPtr subscription_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ConveyorNode>());
    rclcpp::shutdown();
    return 0;
}