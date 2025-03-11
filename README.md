# Original Codebase
https://github.com/ccy-studio/CCY-VFD-7BT-317NK

# CCY-VFD-7BT317NK

This project is for the CCY-VFD-7BT317NK, a VFD desktop clock with various features and configurations.

## Project Structure

### Key Directories and Files

- **assets/**: Contains various text files and data used by the project.
- **data/**: Directory for data files.
- **doc/**: Documentation files.
- **include/**: Header files for the project.
- **lib/**: Project-specific libraries.
- **src/**: Source code for the project.
- **test/**: Unit tests for the project.
- **platformio.ini**: PlatformIO Project Configuration File.
- **index_template.html**: HTML template for the web interface.
- **auto_license.py**: Script to automatically add license headers to files.

## Dependencies

The project uses the following libraries:

- [ArduinoJson](https://github.com/bblanchon/ArduinoJson) (version 7.3.1)
- [WiFiManager](https://github.com/tzapu/WiFiManager)
- [base64](https://github.com/Densaugeo/base64) (version 1.4.0)
- [PubSubClient](https://github.com/knolleary/pubsubclient)

## Configuration

The project is configured using the `platformio.ini` file. Key configurations include:

- **Platform**: espressif8266
- **Board**: esp12e
- **Framework**: arduino
- **Filesystem**: littlefs
- **Upload Port**: `/dev/cu.usbserial-3120`
- **Monitor Speed**: 115200

### OTA Configuration

For OTA (Over-The-Air) updates, the following configuration is used:

- **Upload Protocol**: espota
- **Upload Port**: 192.168.178.39
- **Upload Flags**: `--auth=lonelybinary`

## Building and Uploading

To build and upload the project, use the following commands:

```sh
# Build the project
pio run

# Upload the project via serial
pio run --target upload

# Upload the project via OTA
pio run -e esp12e_ota --target upload