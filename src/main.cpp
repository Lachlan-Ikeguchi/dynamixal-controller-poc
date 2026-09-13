#include "dynamixel_controller.hpp"
#include "dynamixel_sdk/dynamixel_sdk.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// Defaults
const char* DEVICE_NAME = "/dev/ttyUSB0";
int BAUD_RATE = 57600;

// Position-reach polling (10 raw counts worth of radians at 4096 cpr)
const double POSITION_THRESHOLD_RAD = DynamixelController::unitsToRadians(10);
const int MAX_READ_ATTEMPTS = 5;

void printUsage(const char* progName) {
  std::cout << "Usage: " << progName << " [options]\n"
            << "Options:\n"
            << "  --device <path>   Serial port path (default: /dev/ttyUSB0)\n"
            << "  --baud <rate>     Baud rate (default: 57600)\n"
            << "  --help            Show this help message and exit\n";
}

static void printServos(const std::vector<uint8_t>& ids) {
  for (std::size_t i = 0; i < ids.size(); ++i) {
    if (i > 0) std::cout << ',';
    std::cout << " ID " << static_cast<int>(ids[i]);
  }
}

static void printPositions(const std::unordered_map<uint8_t, double>& positions) {
  std::vector<uint8_t> ids;
  ids.reserve(positions.size());
  for (const auto & [id, _] : positions) ids.push_back(id);
  std::sort(ids.begin(), ids.end());
  for (uint8_t id : ids) {
    std::cout << "  ID " << static_cast<int>(id) << ": "
              << positions.at(id) << " rad\n";
  }
}

// Protocol auto-detection using the raw SDK (diagnostic pre-check).
// Returns 0 when Protocol 2.0 is detected, 1 otherwise.
// On success, model_number_out is set to the detected model number.
int detectProtocol(const char* device, int baud, uint16_t& model_number_out) {
  dynamixel::PortHandler* portHandler =
      dynamixel::PortHandler::getPortHandler(device);
  uint8_t dxl_error = 0;
  int dxl_comm_result = COMM_TX_FAIL;

  std::cout << "Opening " << device << " at " << baud << " baud...\n";

  if (!portHandler->openPort()) {
    std::cerr << "error: cannot open port\n"
              << "  1) Port exists: ls " << device << "\n"
              << "  2) Read/write permission: add user to 'dialout' group\n"
              << "  3) No other process is using the port\n";
    return 1;
  }

  if (!portHandler->setBaudRate(baud)) {
    std::cerr << "error: cannot set baud rate\n"
              << "  Try a different rate: 1000000, 115200, 57600, 38400\n";
    portHandler->closePort();
    return 1;
  }

  portHandler->clearPort();
  std::cout << "Detecting protocol...\n";

  // Try Protocol 2.0 first
  dynamixel::PacketHandler* packetHandler2 =
      dynamixel::PacketHandler::getPacketHandler(2.0);
  uint16_t model_number_2 = 0;
  dxl_comm_result = packetHandler2->ping(portHandler, 1, &model_number_2, &dxl_error);

  if (dxl_comm_result == COMM_SUCCESS && dxl_error == 0) {
    model_number_out = model_number_2;
    std::cout << "Protocol 2.0 | Model: " << model_number_2
              << " (0x" << std::hex << model_number_2 << std::dec << ")\n";
    portHandler->closePort();
    return 0;
  }

  // Try Protocol 1.0
  dynamixel::PacketHandler* packetHandler1 =
      dynamixel::PacketHandler::getPacketHandler(1.0);
  uint16_t model_number_1 = 0;
  dxl_comm_result = packetHandler1->ping(portHandler, 1, &model_number_1, &dxl_error);

  if (dxl_comm_result == COMM_SUCCESS && dxl_error == 0) {
    std::cerr << "error: Protocol 1.0 detected (model " << model_number_1
              << ") but this controller only supports Protocol 2.0\n";
    portHandler->closePort();
    return 1;
  }

  std::cerr << "error: no servo responded on " << device << "\n"
            << "  1) Servo is powered\n"
            << "  2) USB adapter wiring: RX->TX, TX->RX, GND->GND\n"
            << "  3) Try a different baud rate (1000000, 115200, 57600, 38400)\n"
            << "  4) Try a different servo ID (broadcast: 0xFE)\n"
            << "  5) USB adapter voltage levels (some need 5V)\n";
  portHandler->closePort();
  return 1;
}

// Poll present positions until all targeted servos are within threshold or max attempts.
void pollPositions(DynamixelController& controller,
                   const std::unordered_map<uint8_t, double>& targets) {
  for (int attempt = 0; attempt < MAX_READ_ATTEMPTS; ++attempt) {
    auto positions = controller.readPositions();
    if (!positions.isSuccess()) {
      std::cerr << "warn: read failed (" << attempt + 1 << "/"
                << MAX_READ_ATTEMPTS << "): "
                << dynamixel::getErrorMessage(positions.error()) << '\n';
      continue;
    }

    printPositions(positions.value());

    bool all_reached = true;
    for (const auto & [id, target_rad] : targets) {
      auto it = positions.value().find(id);
      if (it == positions.value().end() ||
          std::abs(it->second - target_rad) > POSITION_THRESHOLD_RAD) {
        all_reached = false;
        break;
      }
    }

    if (all_reached) {
      std::cout << "Target reached.\n";
      return;
    }
  }
  std::cerr << "warn: target not reached after "
            << MAX_READ_ATTEMPTS << " reads\n";
}

int main(int argc, char** argv)
try {
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
    } else {
      std::cerr << "Unknown or incomplete argument: " << arg << "\n";
      printUsage(argv[0]);
      return 1;
    }
  }

  // Protocol auto-detection (diagnostic pre-check using raw SDK)
  uint16_t model_number = 0;
  if (detectProtocol(DEVICE_NAME, BAUD_RATE, model_number) != 0) {
    return 1;
  }

  // Construct the controller (uses protocol 2.0 internally)
  DynamixelController controller(DEVICE_NAME, BAUD_RATE);

  auto scan = controller.scan();
  if (!scan.isSuccess()) {
    std::cerr << "error: scan failed: "
              << dynamixel::getErrorMessage(scan.error()) << '\n';
    return 1;
  }
  if (scan.value().empty()) {
    std::cerr << "error: no servos found\n";
    return 1;
  }

  std::cout << "Servos:";
  printServos(scan.value());
  std::cout << '\n';

  for (uint8_t id : scan.value()) {
    auto torque = controller.enableTorque(id);
    if (!torque.isSuccess()) {
      std::cerr << "error: torque enable failed for ID "
                << static_cast<int>(id) << ": "
                << dynamixel::getErrorMessage(torque.error()) << '\n';
      return 1;
    }
  }
  std::cout << "Torque enabled.\n\n";

  std::cout << "Commands:\n"
            << "  <radians>   move all servos to angle\n"
            << "  m           multi-servo move\n"
            << "  r           read current positions\n"
            << "  q           quit\n";

  std::string input;
  while (true) {
    std::cout << "> ";
    if (!(std::cin >> input) || input == "q") {
      break;
    }

    if (input == "r") {
      auto positions = controller.readPositions();
      if (positions.isSuccess()) {
        printPositions(positions.value());
      } else {
        std::cerr << "error: read failed: "
                  << dynamixel::getErrorMessage(positions.error()) << '\n';
      }
      continue;
    }

    if (input == "m") {
      std::cout << "Enter 'ID radians' per line. go to move, b to cancel.\n";
      std::unordered_map<uint8_t, double> targets;
      bool go_back = false;

      while (true) {
        std::cout << "  ";
        std::string line;
        std::getline(std::cin >> std::ws, line);
        if (line == "go") {
          break;
        }
        if (line == "b") {
          go_back = true;
          break;
        }

        int id_number = 0;
        double radians = 0.0;
        std::string extra;
        std::istringstream pair(line);
        if (!(pair >> id_number >> radians) || (pair >> extra)) {
          std::cerr << "  invalid pair: " << line << '\n';
          continue;
        }
        if (id_number < 0 || id_number > 252) {
          std::cerr << "  invalid servo ID: " << id_number << '\n';
          continue;
        }
        const auto id = static_cast<uint8_t>(id_number);
        if (std::find(scan.value().begin(), scan.value().end(), id) == scan.value().end()) {
          std::cerr << "  servo ID not found: " << id_number << '\n';
          continue;
        }
        targets[id] = radians;
        std::cout << "  ID " << id_number << " -> " << radians << " rad\n";
      }

      if (go_back) {
        continue;
      }
      if (!targets.empty()) {
        auto result = controller.setTargetPosition(targets);
        if (!result.isSuccess()) {
          std::cerr << "error: move failed: "
                    << dynamixel::getErrorMessage(result.error()) << '\n';
        } else {
          pollPositions(controller, targets);
        }
      }
      continue;
    }

    double target_radians = 0.0;
    try {
      std::size_t parsed = 0;
      target_radians = std::stod(input, &parsed);
      if (parsed != input.size()) {
        throw std::invalid_argument("not a number");
      }
    } catch (const std::exception &) {
      std::cerr << "unknown command. Use <radians>, m, r, or q.\n";
      continue;
    }

    auto result = controller.setTargetPosition(target_radians);
    if (!result.isSuccess()) {
      std::cerr << "error: move failed: "
                << dynamixel::getErrorMessage(result.error()) << '\n';
    } else {
      std::unordered_map<uint8_t, double> targets;
      for (uint8_t id : scan.value()) {
        targets.emplace(id, target_radians);
      }
      pollPositions(controller, targets);
    }
  }

  return 0;
} catch (const std::exception & error) {
  std::cerr << error.what() << '\n';
  return 1;
}
