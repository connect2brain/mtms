//
// Created by alqio on 18.10.2022.
//

#include "optitrack_bridge.h"
rclcpp::Publisher<neuronavigation_interfaces::msg::MotivePositions>::SharedPtr OptitrackBridge::publisher;

void
log_rigid_body_message(geometry_msgs::msg::Transform_<std::allocator<void>> rigid_body, const std::string &entity) {
  auto entity_as_c_string = entity.c_str();
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "\tRigid Body:");
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "\t%s\t%3.2f", entity_as_c_string, rigid_body.translation.x);
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "\t%s\t%3.2f", entity_as_c_string, rigid_body.translation.y);
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "\t%s\t%3.2f", entity_as_c_string, rigid_body.translation.z);
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "\t%s\t%3.2f", entity_as_c_string, rigid_body.rotation.x);
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "\t%s\t%3.2f", entity_as_c_string, rigid_body.rotation.x);
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "\t%s\t%3.2f", entity_as_c_string, rigid_body.rotation.x);
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "\t%s\t%3.2f", entity_as_c_string, rigid_body.rotation.x);
}

geometry_msgs::msg::Transform_<std::allocator<void>> create_ros_transform_from_rigid_body(sRigidBodyData rigid_body_data) {
  geometry_msgs::msg::Transform_<std::allocator<void>> transform;
  transform.translation.x = rigid_body_data.x;
  transform.translation.y = rigid_body_data.y;
  transform.translation.z = rigid_body_data.z;
  transform.rotation.x = rigid_body_data.qx;
  transform.rotation.y = rigid_body_data.qy;
  transform.rotation.z = rigid_body_data.qz;
  transform.rotation.w = rigid_body_data.qw;
  return transform;
}

void OptitrackBridge::data_received_callback(sFrameOfMocapData *data, void *pUserData) {
  auto message = neuronavigation_interfaces::msg::MotivePositions();
  int i = 0;
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "Rigid bodies received: %d", data->nRigidBodies);
  for (i = 0; i < data->nRigidBodies; i++) {
    if (data->RigidBodies[i].ID == 999) {
      message.tool_tip = create_ros_transform_from_rigid_body(data->RigidBodies[i]);
      log_rigid_body_message(message.tool_tip, "Measurement probe");

    } else if (data->RigidBodies[i].ID == 1003) {
      message.head = create_ros_transform_from_rigid_body(data->RigidBodies[i]);
      log_rigid_body_message(message.head, "Head");

    } else if (data->RigidBodies[i].ID == 1002) {
      message.coil = create_ros_transform_from_rigid_body(data->RigidBodies[i]);
      log_rigid_body_message(message.head, "Coil");

    }
  }
  OptitrackBridge::publisher->publish(message);
}

OptitrackBridge::OptitrackBridge() : Node("optitrack_bridge") {
  this->declare_parameter<int>("server_index", 1);
  this->get_parameter("server_index", server_index);

  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "Connecting to motive");

  auto client_created = client.create_client(data_received_callback);

  auto success = client.discover_motive_servers(server_index);
  if (success == 1) {
    RCLCPP_ERROR(rclcpp::get_logger("optitrack_bridge"), "Failed to start optitrack bridge");
    shutdown();
    return;
  }
  client.connect_to_motive();
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "Connected to motive");

  publisher = this->create_publisher<neuronavigation_interfaces::msg::MotivePositions>("/neuronavigation/motive_positions", 10);
}

void OptitrackBridge::shutdown() {
  client.disconnect_client();
}

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<OptitrackBridge>();

  rclcpp::spin(node);
  node->shutdown();

  rclcpp::shutdown();
  return 0;
}
