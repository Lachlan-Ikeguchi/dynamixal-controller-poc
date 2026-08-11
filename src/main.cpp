#include "dynamixel_sdk/dynamixel_sdk.h"
#include <iostream>
#include <cstdlib> // for abs()

int main() {
  dynamixel::PortHandler *portHandler =
      dynamixel::PortHandler::getPortHandler("/dev/ttyUSB0");
  dynamixel::PacketHandler *packetHandler =
      dynamixel::PacketHandler::getPacketHandler(2.0);
  if (portHandler->openPort()) {
    std::cout << "Succeeded to open the port!\n";
  } else {
    std::cout << "[ERROR] Failed to open the port!\n";
    std::cout << "Check: 1) Port exists: ls /dev/ttyUSB0\n";
    std::cout << "       2) You have read/write permissions: add user to 'dialout' group\n";
    std::cout << "       3) No other process is using the port\n";
    return 0;
  }

  if (portHandler->setBaudRate(57600)) {
    std::cout << "Succeeded to change the baudrate!\n";
  } else {
    std::cout << "[ERROR] Failed to change the baudrate!\n";
    std::cout << "Check: Your servo might need a different baud rate.\n";
    return 0;
  }

  // Clear port buffer before starting
  portHandler->clearPort();

  // Set packet timeout to 500ms (increased from 100ms for more reliability)
  portHandler->setPacketTimeout(500.0);

  uint8_t dxl_id = 1;
  uint16_t torque_on_address = 64;
  uint8_t data = 1;
  uint8_t dxl_error = 0;
  int dxl_comm_result = COMM_SUCCESS;

  // First, try to ping the servo to verify communication
  std::cout << "Pinging Dynamixel ID " << (int)dxl_id << "...\n";
  uint16_t model_number = 0;
  dxl_comm_result = packetHandler->ping(portHandler, dxl_id, &model_number, &dxl_error);
  if (dxl_comm_result != COMM_SUCCESS) {
    std::cout << "[ERROR] Ping failed: " << packetHandler->getTxRxResult(dxl_comm_result) << std::endl;
    std::cout << "This means the servo is not responding.\n";
    std::cout << "Check: 1) Servo is powered, 2) USB adapter is properly connected (RX->TX, TX->RX, GND->GND),\n";
    std::cout << "       3) Try a different baud rate (1000000, 115200, 57600, 38400),\n";
    std::cout << "       4) Correct servo ID (try broadcast ping with ID 0xFE),\n";
    std::cout << "       5) USB adapter voltage levels (some need 5V, servo TX is 5V)\n";
    return 0;
  } else if (dxl_error != 0) {
    std::cout << "[ERROR] Ping hardware error: " << packetHandler->getRxPacketError(dxl_error) << std::endl;
    return 0;
  } else {
    std::cout << "Ping successful! Model number: " << model_number << " (0x" << std::hex << model_number << std::dec << ")\n";
  }

  std::cout << "Attempting to enable torque on Dynamixel ID " << (int)dxl_id << "...\n";
  dxl_comm_result = packetHandler->write1ByteTxRx(
      portHandler, dxl_id, torque_on_address, data, &dxl_error);
  if (dxl_comm_result != COMM_SUCCESS) {
    std::cout << "[ERROR] Communication failed: " << packetHandler->getTxRxResult(dxl_comm_result) << std::endl;
    std::cout << "Check: 1) Servo is powered, 2) Correct serial port, 3) Correct baud rate (57600), 4) Correct servo ID (" << (int)dxl_id << ")\n";
    return 0;
  } else if (dxl_error != 0) {
    std::cout << "[ERROR] Hardware error: " << packetHandler->getRxPacketError(dxl_error) << std::endl;
    return 0;
  } else {
    std::cout << "Dynamixel#" << (int)dxl_id << " has been successfully connected \n";
  }

  int target_position;
  while (true) {
    std::cout << "Enter target position (0 ~ 4095): ";
    std::cin >> target_position;
    if (target_position == -1) {
      break; // Exit on -1 input
    } else if (target_position < 0 || target_position > 4095) {
      std::cout << "Position must be between 0 and 4095." << std::endl;
      continue;
    }

    uint16_t goal_position_address = 116;
    dxl_comm_result = packetHandler->write4ByteTxRx(
        portHandler, dxl_id, goal_position_address, uint32_t(target_position),
        &dxl_error);
    if (dxl_comm_result != COMM_SUCCESS) {
      std::cout << "[ERROR] Failed to set target position: " << packetHandler->getTxRxResult(dxl_comm_result) << std::endl;
      continue;
    } else if (dxl_error != 0) {
      std::cout << "[ERROR] Hardware error setting position: " << packetHandler->getRxPacketError(dxl_error) << std::endl;
      continue;
    }

    uint16_t present_position_address = 132;
    uint32_t present_position;
    int read_attempts = 0;
    const int max_read_attempts = 5;
    do {
      dxl_comm_result = packetHandler->read4ByteTxRx(
          portHandler, dxl_id, present_position_address, &present_position,
          &dxl_error);
      if (dxl_comm_result != COMM_SUCCESS) {
        std::cout << "[WARNING] Failed to read position (attempt " << ++read_attempts << "/" << max_read_attempts << "): " << packetHandler->getTxRxResult(dxl_comm_result) << std::endl;
        if (read_attempts >= max_read_attempts) {
          std::cout << "[ERROR] Too many read failures, exiting...\n";
          break;
        }
      } else if (dxl_error != 0) {
        std::cout << "[ERROR] Hardware error reading position: " << packetHandler->getRxPacketError(dxl_error) << std::endl;
        break;
      } else {
        read_attempts = 0; // Reset on success
        std::cout << "Current Position: " << present_position << std::endl;
      }
    } while (abs(static_cast<int>(target_position - present_position)) > 10);
  }

  // Disable torque and clean up
  data = 0;
  dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, dxl_id, torque_on_address, data, &dxl_error);
  if (dxl_comm_result != COMM_SUCCESS) {
    std::cout << "[WARNING] Failed to disable torque: " << packetHandler->getTxRxResult(dxl_comm_result) << std::endl;
  }
  
  portHandler->closePort();
  std::cout << "Port closed. Exiting...\n";

  return 0;
}
