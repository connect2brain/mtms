//
// Created by alqio on 18.10.2022.
//

#include "optitrack_bridge.h"

rclcpp::Publisher<neuronavigation_interfaces::msg::OptitrackPoses>::SharedPtr OptitrackBridge::publisher;

void log_rigid_body_message(geometry_msgs::msg::Transform rigid_body, const std::string &entity) {
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"),
              "%s: Translation (x, y, z): (%3.2f, %3.2f, %3.2f), Rotation: (x, y, z, w): (%3.2f, %3.2f, %3.2f, %3.2f)",
              entity.c_str(),
              rigid_body.translation.x,
              rigid_body.translation.y,
              rigid_body.translation.z,
              rigid_body.rotation.x,
              rigid_body.rotation.y,
              rigid_body.rotation.z,
              rigid_body.rotation.w
  );
}

geometry_msgs::msg::Transform create_ros_transform_from_rigid_body(sRigidBodyData rigid_body_data) {
  geometry_msgs::msg::Transform transform;
  transform.translation.x = rigid_body_data.x;
  transform.translation.y = rigid_body_data.y;
  transform.translation.z = rigid_body_data.z;
  transform.rotation.x = rigid_body_data.qx;
  transform.rotation.y = rigid_body_data.qy;
  transform.rotation.z = rigid_body_data.qz;
  transform.rotation.w = rigid_body_data.qw;
  return transform;
}

void OptitrackBridge::data_received_callback(sFrameOfMocapData *data, void *user_data) {
  auto message = neuronavigation_interfaces::msg::OptitrackPoses();
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "Rigid bodies received: %d", data->nRigidBodies);
  for (int i = 0; i < data->nRigidBodies; i++) {
    if (data->RigidBodies[i].ID == MEASUREMENT_PROBE_RIGID_BODY_ID) {
      message.probe = create_ros_transform_from_rigid_body(data->RigidBodies[i]);
      log_rigid_body_message(message.probe, "Measurement probe");

    } else if (data->RigidBodies[i].ID == HEAD_RIGID_BODY_ID) {
      message.head = create_ros_transform_from_rigid_body(data->RigidBodies[i]);
      log_rigid_body_message(message.head, "Head");

    } else if (data->RigidBodies[i].ID == COIL_RIGID_BODY_ID) {
      message.coil = create_ros_transform_from_rigid_body(data->RigidBodies[i]);
      log_rigid_body_message(message.head, "Coil");

    }
  }
  OptitrackBridge::publisher->publish(message);
}

OptitrackBridge::OptitrackBridge() : Node("optitrack_bridge") {
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "Connecting to Motive...");

  auto client_created = client.create_client(data_received_callback);
  if (client_created != 0) {
    RCLCPP_ERROR(rclcpp::get_logger("optitrack_bridge"),
                 "Failed to initialize NatNet Client. Failed to start Optitrack bridge.");
    return;
  }

  auto success = client.discover_motive_servers();
  if (success != 0) {
    RCLCPP_ERROR(rclcpp::get_logger("optitrack_bridge"), "Failed to start Optitrack bridge.");
    shutdown();
    return;
  }
  client.connect_to_motive();
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "Connected to Motive.");

  publisher = this->create_publisher<neuronavigation_interfaces::msg::OptitrackPoses>(
      "/neuronavigation/optitrack_poses", 1);
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
