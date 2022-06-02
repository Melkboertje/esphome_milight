#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c_esp32/i2c_esp32.h"

namespace esphome {
namespace sht3xd_esp32 {

/// This class implements support for the SHT3x-DIS family of temperature+humidity i2c sensors.
class SHT3XDComponent : public PollingComponent, public i2c_esp32::I2CDevice {
 public:
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { temperature_sensor_ = temperature_sensor; }
  void set_humidity_sensor(sensor::Sensor *humidity_sensor) { humidity_sensor_ = humidity_sensor; }

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void update() override;
  
  enum CommandLen : uint8_t { ADDR_8_BIT = 1, ADDR_16_BIT = 2 };

  /** Read data words from i2c device.
   * handles crc check used by Sensirion sensors
   * @param data pointer to raw result
   * @param len number of words to read
   * @return true if reading succeded
   */
  bool read_data(uint16_t *data, uint8_t len);

  /** Read 1 data word from i2c device.
   * @param data reference to raw result
   * @return true if reading succeded
   */
  bool read_data(uint16_t &data) { return this->read_data(&data, 1); }

  /** get data words from i2c register.
   * handles crc check used by Sensirion sensors
   * @param  i2c register
   * @param data pointer to raw result
   * @param len number of words to read
   * @param delay milliseconds to to wait between sending the i2c commmand and reading the result
   * @return true if reading succeded
   */
  bool get_register(uint16_t command, uint16_t *data, uint8_t len, uint8_t delay = 0) {
    return get_register_(command, ADDR_16_BIT, data, len, delay);
  }
  /** Read 1 data word from 16 bit i2c register.
   * @param  i2c register
   * @param data reference to raw result
   * @param delay milliseconds to to wait between sending the i2c commmand and reading the result
   * @return true if reading succeded
   */
  bool get_register(uint16_t i2c_register, uint16_t &data, uint8_t delay = 0) {
    return this->get_register_(i2c_register, ADDR_16_BIT, &data, 1, delay);
  }

  /** get data words from i2c register.
   * handles crc check used by Sensirion sensors
   * @param  i2c register
   * @param data pointer to raw result
   * @param len number of words to read
   * @param delay milliseconds to to wait between sending the i2c commmand and reading the result
   * @return true if reading succeded
   */
  bool get_8bit_register(uint8_t i2c_register, uint16_t *data, uint8_t len, uint8_t delay = 0) {
    return get_register_(i2c_register, ADDR_8_BIT, data, len, delay);
  }

  /** Read 1 data word from 8 bit i2c register.
   * @param  i2c register
   * @param data reference to raw result
   * @param delay milliseconds to to wait between sending the i2c commmand and reading the result
   * @return true if reading succeded
   */
  bool get_8bit_register(uint8_t i2c_register, uint16_t &data, uint8_t delay = 0) {
    return this->get_register_(i2c_register, ADDR_8_BIT, &data, 1, delay);
  }

  /** Write a command to the i2c device.
   * @param command i2c command to send
   * @return true if reading succeded
   */
  template<class T> bool write_command(T i2c_register) { return write_command(i2c_register, nullptr, 0); }

  /** Write a command and one data word to the i2c device .
   * @param command i2c command to send
   * @param data argument for the i2c command
   * @return true if reading succeded
   */
  template<class T> bool write_command(T i2c_register, uint16_t data) { return write_command(i2c_register, &data, 1); }

  /** Write a command with arguments as words
   * @param i2c_register i2c command to send - an be uint8_t or uint16_t
   * @param data vector<uint16> arguments for the i2c command
   * @return true if reading succeded
   */
  template<class T> bool write_command(T i2c_register, const std::vector<uint16_t> &data) {
    return write_command_(i2c_register, sizeof(T), data.data(), data.size());
  }

  /** Write a command with arguments as words
   * @param i2c_register i2c command to send - an be uint8_t or uint16_t
   * @param data arguments for the i2c command
   * @param len number of arguments (words)
   * @return true if reading succeded
   */
  template<class T> bool write_command(T i2c_register, const uint16_t *data, uint8_t len) {
    // limit to 8 or 16 bit only
    static_assert(sizeof(i2c_register) == 1 || sizeof(i2c_register) == 2,
                  "only 8 or 16 bit command types are supported.");
    return write_command_(i2c_register, CommandLen(sizeof(T)), data, len);
  }

 protected:
  uint8_t crc_polynomial_{0x31u};  // default for sensirion
  /** Write a command with arguments as words
   * @param command i2c command to send can be uint8_t or uint16_t
   * @param command_len either 1 for short 8 bit command or 2 for 16 bit command codes
   * @param data arguments for the i2c command
   * @param data_len number of arguments (words)
   * @return true if reading succeded
   */
  bool write_command_(uint16_t command, CommandLen command_len, const uint16_t *data, uint8_t data_len);

  /** get data words from i2c register.
   * handles crc check used by Sensirion sensors
   * @param  i2c register
   * @param command_len either 1 for short 8 bit command or 2 for 16 bit command codes
   * @param data pointer to raw result
   * @param len number of words to read
   * @param delay milliseconds to to wait between sending the i2c commmand and reading the result
   * @return true if reading succeded
   */
  bool get_register_(uint16_t reg, CommandLen command_len, uint16_t *data, uint8_t len, uint8_t delay);

  /** 8-bit CRC checksum that is transmitted after each data word for read and write operation
   * @param command i2c command to send
   * @param data data word for which the crc8 checksum is calculated
   * @param len number of arguments (words)
   * @return 8 Bit CRC
   */
  uint8_t sht_crc_(uint16_t data);

  /** 8-bit CRC checksum that is transmitted after each data word for read and write operation
   * @param command i2c command to send
   * @param data1 high byte of data word
   * @param data2 low byte of data word
   * @return 8 Bit CRC
   */
  uint8_t sht_crc_(uint8_t data1, uint8_t data2) { return sht_crc_(encode_uint16(data1, data2)); }

  /** last error code from i2c operation
   */
  i2c_esp32::ErrorCode last_error_;
  
 protected:
  sensor::Sensor *temperature_sensor_;
  sensor::Sensor *humidity_sensor_;
};

}  // namespace sht3xd_esp32
}  // namespace esphome