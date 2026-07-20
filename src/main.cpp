#include <Arduino.h>
#include <Dynamixel2Arduino.h>

using namespace ControlTableItem;

HardwareSerial SerialPort(1);

const uint8_t DXL_ID = 1;
const float DXL_PROTOCOL_VERSION = 2.0;

const int DXL_DIR_PIN = 18; // Change to suite real hardware.

extern "C" void app_main() {
    SerialPort.begin(9600, SERIAL_8N1, 16, 17); // RX, TX pins.  Change to suit real hardware.

    Dynamixel2Arduino dxl(SerialPort, DXL_DIR_PIN);

    dxl.begin(57600); // Must match dynamixel's baud rate
    
    dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

    dxl.ping(DXL_ID);
}
