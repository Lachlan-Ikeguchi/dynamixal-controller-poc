#include "dynamixel_sdk/dynamixel_sdk.h"
#include <iostream>

int main(){
  dynamixel::PortHandler *portHandler = dynamixel::PortHandler::getPortHandler("/dev/ttyUSB0");
  dynamixel::PacketHandler *packetHandler = dynamixel::PacketHandler::getPacketHandler(2.0);
  if (portHandler->openPort()) {
    std::cout << "Succeeded to open the port!\n";
  } else {
    std::cout << "Failed to open the port!\n";
    return 0;
  }

  if (portHandler->setBaudRate(57600)) {
    std::cout << "Succeeded to change the baudrate!\n";
  } else {
    std::cout << "Failed to change the baudrate!\n";
    return 0;
  }

  uint8_t dxl_id = 1;
  uint16_t torque_on_address = 64;
  uint8_t data = 1;
  uint8_t dxl_error = 0;
  int dxl_comm_result = COMM_TX_FAIL;

  dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, dxl_id, torque_on_address, data, &dxl_error);
  if (dxl_comm_result != COMM_SUCCESS) {
    std::cout << packetHandler->getTxRxResult(dxl_comm_result) << std::endl;
  } else if (dxl_error != 0) {
    std::cout << packetHandler->getRxPacketError(dxl_error) << std::endl;
  } else {
    std::cout << "Dynamixel#1 has been successfully connected \n";
  }

  int target_position;
  while(true){
    std::cout << "Enter target position (0 ~ 4095): ";
    std::cin >> target_position;
    if(target_position == -1){
      break; // Exit on -1 input
    } else if(target_position < 0 || target_position > 4095){
      std::cout << "Position must be between 0 and 4095." << std::endl;
      continue;
    }

    uint16_t goal_position_address = 116;
    dxl_comm_result = packetHandler->write4ByteTxRx(portHandler, dxl_id, goal_position_address, uint32_t(target_position), &dxl_error);
    if (dxl_comm_result != COMM_SUCCESS) {
      std::cout << packetHandler->getTxRxResult(dxl_comm_result) << std::endl;
    } else if (dxl_error != 0) {
      std::cout << packetHandler->getRxPacketError(dxl_error) << std::endl;
    }

    uint16_t present_position_address = 132;
    uint32_t present_position;
    do {
      dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, dxl_id, present_position_address, &present_position, &dxl_error);
      if (dxl_comm_result != COMM_SUCCESS) {
        std::cout << packetHandler->getTxRxResult(dxl_comm_result) << std::endl;
      } else if (dxl_error != 0) {
        std::cout << packetHandler->getRxPacketError(dxl_error) << std::endl;
      }
      std::cout << "Current Position: " << present_position << std::endl;
    } while (abs(static_cast<int>(target_position - present_position)) > 10);
  }

  data = 0;
  packetHandler->write1ByteTxRx(portHandler, dxl_id, torque_on_address, data);
  portHandler->closePort();

  return 0;
}
