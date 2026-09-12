#include "dynamixel_sdk/dynamixel_sdk.h"
#include <iostream>
#include <cstdlib> // for abs()
#include <string>

// Configuration defaults
const char* DEVICE_NAME = "/dev/ttyUSB0";
int BAUD_RATE = 57600;
uint8_t DXL_ID = 1;
const uint16_t TORQUE_ENABLE_ADDRESS = 64;
const uint16_t GOAL_POSITION_ADDRESS = 116;
const uint16_t PRESENT_POSITION_ADDRESS = 132;
const int POSITION_THRESHOLD = 10;
const int MAX_READ_ATTEMPTS = 5;

void printUsage(const char* progName) {
  std::cout << "Usage: " << progName << " [options]\n"
            << "Options:\n"
            << "  --device <path>   Serial port path (default: /dev/ttyUSB0)\n"
            << "  --baud <rate>      Baud rate (default: 57600)\n"
            << "  --id <n>           Dynamixel servo ID (default: 1)\n"
            << "  --help             Show this help message and exit\n";
}

int main(int argc, char** argv) {
  // Parse CLI arguments
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--help") {
      printUsage(argv[0]);
      return 0;
    } else if (arg == "--device" && i + 1 < argc) {
      DEVICE_NAME = argv[++i];
    } else if (arg == "--baud" && i + 1 < argc) {
      BAUD_RATE = std::atoi(argv[++i]);
    } else if (arg == "--id" && i + 1 < argc) {
      DXL_ID = static_cast<uint8_t>(std::atoi(argv[++i]));
    } else {
      std::cerr << "Unknown or incomplete argument: " << arg << "\n";
      printUsage(argv[0]);
      return 1;
    }
  }

  // Setup port handler
  dynamixel::PortHandler *portHandler =
      dynamixel::PortHandler::getPortHandler(DEVICE_NAME);

  uint8_t dxl_error = 0;
  int dxl_comm_result = COMM_TX_FAIL;
  uint16_t model_number = 0;
  dynamixel::PacketHandler *packetHandler = nullptr;

  // Open port
  if (!portHandler->openPort()) {
    std::cout << "[ERROR] Failed to open the port!\n";
    std::cout << "Check: 1) Port exists: ls " << DEVICE_NAME << "\n";
    std::cout << "       2) You have read/write permissions: add user to 'dialout' group\n";
    std::cout << "       3) No other process is using the port\n";
    return 1;
  }
  std::cout << "Succeeded to open the port!\n";

  // Set baud rate
  if (!portHandler->setBaudRate(BAUD_RATE)) {
    std::cout << "[ERROR] Failed to change the baudrate!\n";
    std::cout << "Check: Your servo might need a different baud rate.\n";
    std::cout << "Common rates: 1000000, 115200, 57600, 38400\n";
    portHandler->closePort();
    return 1;
  }
  std::cout << "Succeeded to change the baudrate!\n";

  // Clear port buffer
  portHandler->clearPort();

  // Auto-detect protocol (MX-106R with 2.0 firmware most likely uses Protocol 2.0)
  std::cout << "Auto-detecting servo protocol...\n";

  // Try Protocol 2.0 first
  packetHandler = dynamixel::PacketHandler::getPacketHandler(2.0);
  uint16_t model_number_2 = 0;
  dxl_comm_result = packetHandler->ping(portHandler, DXL_ID, &model_number_2, &dxl_error);

  if (dxl_comm_result == COMM_SUCCESS && dxl_error == 0) {
    model_number = model_number_2;
    std::cout << "Detected Protocol: 2.0\n";
  } else {
    // Print diagnostic for the Protocol 2.0 failure
    std::cout << "Protocol 2.0 ping failed:\n";
    std::cout << "  Comm result: " << packetHandler->getTxRxResult(dxl_comm_result) << std::endl;
    if (dxl_error != 0)
      std::cout << "  Hardware error: " << packetHandler->getRxPacketError(dxl_error) << std::endl;

    // Try Protocol 1.0
    packetHandler = dynamixel::PacketHandler::getPacketHandler(1.0);
    uint16_t model_number_1 = 0;
    dxl_comm_result = packetHandler->ping(portHandler, DXL_ID, &model_number_1, &dxl_error);

    if (dxl_comm_result == COMM_SUCCESS && dxl_error == 0) {
      model_number = model_number_1;
      std::cout << "Detected Protocol: 1.0\n";
    } else {
      std::cout << "Protocol 1.0 ping failed:\n";
      std::cout << "  Comm result: " << packetHandler->getTxRxResult(dxl_comm_result) << std::endl;
      if (dxl_error != 0)
        std::cout << "  Hardware error: " << packetHandler->getRxPacketError(dxl_error) << std::endl;

      std::cout << "[ERROR] Failed to detect protocol - neither 1.0 nor 2.0 worked\n";
      std::cout << "Check: 1) Servo is powered\n";
      std::cout << "       2) USB adapter is properly connected (RX->TX, TX->RX, GND->GND)\n";
      std::cout << "       3) Try a different baud rate (1000000, 115200, 57600, 38400)\n";
      std::cout << "       4) Correct servo ID (try broadcast ping with ID 0xFE)\n";
      std::cout << "       5) USB adapter voltage levels (some need 5V, servo TX is 5V)\n";
      portHandler->closePort();
      return 1;
    }
  }

  std::cout << "Ping successful! Model number: " << model_number 
            << " (0x" << std::hex << model_number << std::dec << ")\n";

  // Enable torque on the servo
  std::cout << "Attempting to enable torque on Dynamixel ID " 
            << static_cast<int>(DXL_ID) << "...\n";
  uint8_t torque_enable_data = 1;
  dxl_comm_result = packetHandler->write1ByteTxRx(
      portHandler, DXL_ID, TORQUE_ENABLE_ADDRESS, torque_enable_data, &dxl_error);

  if (dxl_comm_result != COMM_SUCCESS) {
    std::cout << "[ERROR] Communication failed: " 
              << packetHandler->getTxRxResult(dxl_comm_result) << std::endl;
    std::cout << "Check: 1) Servo is powered, 2) Correct serial port, "
              << "3) Correct baud rate (" << BAUD_RATE << "), "
              << "4) Correct servo ID (" << static_cast<int>(DXL_ID) << ")\n";
    portHandler->closePort();
    return 1;
  } else if (dxl_error != 0) {
    std::cout << "[ERROR] Hardware error: " 
              << packetHandler->getRxPacketError(dxl_error) << std::endl;
    portHandler->closePort();
    return 1;
  } else {
    std::cout << "Dynamixel#" << static_cast<int>(DXL_ID) 
              << " has been successfully connected\n";
  }

  // Position control loop
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

    // Set goal position
    dxl_comm_result = packetHandler->write4ByteTxRx(
        portHandler, DXL_ID, GOAL_POSITION_ADDRESS, 
        static_cast<uint32_t>(target_position), &dxl_error);

    if (dxl_comm_result != COMM_SUCCESS) {
      std::cout << "[ERROR] Failed to set target position: " 
                << packetHandler->getTxRxResult(dxl_comm_result) << std::endl;
      continue;
    } else if (dxl_error != 0) {
      std::cout << "[ERROR] Hardware error setting position: " 
                << packetHandler->getRxPacketError(dxl_error) << std::endl;
      continue;
    }

    // Read present position until we reach the target (with retry limit)
    uint32_t present_position = 0;
    int read_attempts = 0;
    bool position_reached = false;

    while (read_attempts < MAX_READ_ATTEMPTS) {
      dxl_comm_result = packetHandler->read4ByteTxRx(
          portHandler, DXL_ID, PRESENT_POSITION_ADDRESS, 
          &present_position, &dxl_error);

      if (dxl_comm_result != COMM_SUCCESS) {
        read_attempts++;
        std::cout << "[WARNING] Failed to read position (attempt " 
                  << read_attempts << "/" << MAX_READ_ATTEMPTS << "): "
                  << packetHandler->getTxRxResult(dxl_comm_result) << std::endl;
        if (read_attempts >= MAX_READ_ATTEMPTS) {
          std::cout << "[ERROR] Too many read failures, skipping to next command.\n";
          break;
        }
      } else if (dxl_error != 0) {
        std::cout << "[ERROR] Hardware error reading position: " 
                  << packetHandler->getRxPacketError(dxl_error) << std::endl;
        break;
      } else {
        read_attempts = 0; // Reset on success
        std::cout << "Current Position: " << present_position << std::endl;

        // Check if we've reached the target position (within threshold)
        if (std::abs(static_cast<int>(target_position - present_position)) 
            <= POSITION_THRESHOLD) {
          position_reached = true;
          break;
        }
      }
    }

    if (!position_reached && read_attempts >= MAX_READ_ATTEMPTS) {
      std::cout << "[WARNING] Position may not have been reached.\n";
    }
  }

  // Disable torque and clean up
  std::cout << "Disabling torque...\n";
  uint8_t torque_disable_data = 0;
  dxl_comm_result = packetHandler->write1ByteTxRx(
      portHandler, DXL_ID, TORQUE_ENABLE_ADDRESS, torque_disable_data, &dxl_error);

  if (dxl_comm_result != COMM_SUCCESS) {
    std::cout << "[WARNING] Failed to disable torque: " 
              << packetHandler->getTxRxResult(dxl_comm_result) << std::endl;
  }

  portHandler->closePort();
  std::cout << "Port closed. Exiting...\n";

  return 0;
}
