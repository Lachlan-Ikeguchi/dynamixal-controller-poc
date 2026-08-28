#include "dynamixel_controller.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

int main()
try {
  DynamixelController controller("/dev/ttyUSB0", 57600);

  auto scan = controller.scan();
  if (!scan.isSuccess()) {
    std::cerr << "Scan failed: " << dynamixel::getErrorMessage(scan.error()) << '\n';
    return 1;
  }
  if (scan.value().empty()) {
    std::cerr << "No servos found.\n";
    return 1;
  }

  std::cout << "Found servo IDs:";
  for (uint8_t id : scan.value()) {
    std::cout << ' ' << static_cast<int>(id);
  }
  std::cout << '\n';

  for (uint8_t id : scan.value()) {
    auto torque = controller.enableTorque(id);
    if (!torque.isSuccess()) {
      std::cerr << "Torque failed for ID " << static_cast<int>(id) << ": "
                << dynamixel::getErrorMessage(torque.error()) << '\n';
      return 1;
    }
  }

  std::string input;
  while (true) {
    std::cout << "Enter target radians, m for multiple, r to read, or q to exit: ";
    if (!(std::cin >> input) || input == "q") {
      break;
    }

    if (input == "r") {
      auto positions = controller.readPositions();
      if (positions.isSuccess()) {
        std::cout << "Current positions:";
        for (const auto & [id, radians] : positions.value()) {
          std::cout << " [ID " << static_cast<int>(id) << ": " << radians << " rad]";
        }
        std::cout << '\n';
      } else {
        std::cerr << "Read failed: "
                  << dynamixel::getErrorMessage(positions.error()) << '\n';
      }
      continue;
    }

    if (input == "m") {
      std::cout << "Enter one 'ID radians' pair per line. Type go to move or b to go back.\n";
      std::unordered_map<uint8_t, double> targets;
      std::vector<std::string> errors;
      bool go_back = false;

      while (true) {
        std::cout << "> ";
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
          errors.push_back("Invalid pair: " + line);
          continue;
        }
        if (id_number < 0 || id_number > 252) {
          errors.push_back("Invalid servo ID: " + std::to_string(id_number));
          continue;
        }
        const auto id = static_cast<uint8_t>(id_number);
        if (std::find(scan.value().begin(), scan.value().end(), id) == scan.value().end()) {
          errors.push_back("Servo ID not found: " + std::to_string(id_number));
          continue;
        }
        targets[id] = radians;
      }

      if (go_back) {
        continue;
      }
      if (!targets.empty()) {
        auto result = controller.setTargetPosition(targets);
        if (!result.isSuccess()) {
          errors.push_back("Move failed: " + dynamixel::getErrorMessage(result.error()));
        }
      }

      for (const auto & error : errors) {
        std::cerr << error << '\n';
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
      std::cout << "Enter radians, m, r, or q.\n";
      continue;
    }

    auto result = controller.setTargetPosition(target_radians);
    if (!result.isSuccess()) {
      std::cerr << "Move failed: " << dynamixel::getErrorMessage(result.error()) << '\n';
    }
  }

  return 0;
} catch (const std::exception & error) {
  std::cerr << error.what() << '\n';
  return 1;
}
