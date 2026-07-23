#include <Arduino.h>
#include <Dynamixel2Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microros/rmw_microros.h>

#include <std_msgs/msg/float32_multi_array.h>

#define DXL_SERIAL   Serial1
#define DXL_DIR_PIN  -1    
#define DXL_PROTOCOL 2.0
#define BAUDRATE     57600 
#define NUM_DXL      8

Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);
uint8_t dxl_id[] = {1, 2, 3, 4, 5, 6, 7, 8};

rcl_subscription_t hand_sub;
std_msgs__msg__Float32MultiArray hand_msg;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

#define LED_PIN LED_BUILTIN

uint32_t degToPos(double deg) {
  return (uint32_t)(deg * 4095.0 / 360.0);
}

// ================= CALLBACK =================
void hand_callback(const void * msgin) {
  const std_msgs__msg__Float32MultiArray * msg = (const std_msgs__msg__Float32MultiArray *)msgin;
  
  if (msg->data.size >= NUM_DXL) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));

    for (uint8_t i = 0; i < NUM_DXL; i++) {
      dxl.setGoalPosition(dxl_id[i], degToPos(msg->data.data[i]));
    }
  }
}

// ================= SETUP =================
void setup() {
  pinMode(LED_PIN, OUTPUT);

  dxl.begin(BAUDRATE);
  dxl.setPortProtocolVersion(DXL_PROTOCOL);
  for (int i = 0; i < NUM_DXL; i++) {
    dxl.torqueOff(dxl_id[i]);
    dxl.setOperatingMode(dxl_id[i], OP_CURRENT_BASED_POSITION);
    dxl.torqueOn(dxl_id[i]);
    dxl.setGoalCurrent(dxl_id[i], 250);
  }

  set_microros_serial_transports(Serial);
  delay(2000);

  allocator = rcl_get_default_allocator();
  while (RMW_RET_OK != rmw_uros_ping_agent(100, 10)) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(100);
  }
  digitalWrite(LED_PIN, HIGH);

  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_default(&node, "hand_node", "", &support);

  std_msgs__msg__Float32MultiArray__init(&hand_msg);
  hand_msg.data.data = (float *)malloc(NUM_DXL * sizeof(float));
  hand_msg.data.capacity = NUM_DXL;
  hand_msg.data.size = 0;

  rclc_subscription_init_default(
    &hand_sub, &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32MultiArray), 
    "/hand_controller");

  executor = rclc_executor_get_zero_initialized_executor();
  rclc_executor_init(&executor, &support.context, 1, &allocator);
  rclc_executor_add_subscription(&executor, &hand_sub, &hand_msg, &hand_callback, ON_NEW_DATA);
}

void loop() {
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
  
  static unsigned long last_sync = 0;
  if (millis() - last_sync > 100) {
    (void) rmw_uros_sync_session(100);
    last_sync = millis();
  }
}
