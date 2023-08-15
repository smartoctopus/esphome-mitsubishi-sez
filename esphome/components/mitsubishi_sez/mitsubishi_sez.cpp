#include "mitsubishi_sez.h"
#include "cstdint"
#include "esphome/core/log.h"

namespace esphome {
namespace mitsubishi_sez {
static char const *TAG = "mitsubishi_sez.climate";

// Power
const uint32_t OFF = 0x00;
const uint32_t ON = 0x40;

// Operating mode
const uint8_t AUTO = 0x03;
const uint8_t HEAT = 0x02;
const uint8_t COOL = 0x01;
const uint8_t DRY = 0x05;
const uint8_t FAN = 0x00;

// Fan speed
const uint8_t FAN1 = 0x31;
const uint8_t FAN2 = 0x33;
const uint8_t FAN3 = 0x35;
const uint8_t FAN4 = 0x37;

// Pulse parameters in usec
const uint16_t HEADER_MARK = 3060;
const uint16_t HEADER_SPACE = 1580;
const uint16_t BIT_MARK = 350;
const uint16_t ONE_SPACE = 1150;
const uint16_t ZERO_SPACE = 390;

#define MSG_LENGTH 17

bool MitsubishiSEZClimate::on_receive(remote_base::RemoteReceiveData data)
{
    ESP_LOGD(TAG, "Received some bytes");

    uint8_t bytes[MSG_LENGTH] = {};

    if (!data.expect_item(HEADER_MARK, HEADER_SPACE))
        return false;

    for (uint8_t a_byte = 0; a_byte < MSG_LENGTH; a_byte++) {
        uint8_t byte = 0;
        for (int8_t a_bit = 0; a_bit < 8; a_bit++) {
            if (data.expect_item(BIT_MARK, ONE_SPACE))
                byte |= 1 << a_bit;
            else if (!data.expect_item(BIT_MARK, ZERO_SPACE))
                return false;
        }
        bytes[a_byte] = byte;
    }

    ESP_LOGD(TAG,
        "Received bytes 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X",
        bytes[0], bytes[1], bytes[2], bytes[3],
        bytes[4], bytes[5], bytes[6], bytes[7],
        bytes[8], bytes[9], bytes[10], bytes[11],
        bytes[12], bytes[13], bytes[14], bytes[15],
        bytes[16]);

    // Check the static bytes
    if (bytes[0] != 0x23 || bytes[1] != 0xCB || bytes[2] != 0x26 || bytes[3] != 0x21 || bytes[4] != 0x00) {
        return false;
    }

    ESP_LOGD(TAG, "Passed check static bytes");

#define CHECK_BYTE(a, b) (bytes[a] != (~bytes[b] & 0xFF))

    if (CHECK_BYTE(11, 5) || CHECK_BYTE(12, 6) || CHECK_BYTE(13, 7) || CHECK_BYTE(14, 8) || CHECK_BYTE(15, 9) || CHECK_BYTE(16, 10)) {
        return false;
    }

#undef CHECK_BYTE

    ESP_LOGD(TAG, "Passed check inversed bytes");

    auto powerMode = bytes[5];
    auto operationMode = bytes[6] & 0x0F;
    auto temperature = (bytes[6] >> 4) + 16;
    auto fanSpeed = bytes[7];

    ESP_LOGD(TAG,
        "Resulting numbers: powerMode=0x%02X operationMode=0x%02X temperature=%d fanSpeed=0x%02X",
        powerMode, operationMode, temperature, fanSpeed);

    if (powerMode == ON) {
        // Power and operating mode
        switch (operationMode) {
        case COOL:
            this->mode = climate::CLIMATE_MODE_COOL;
            break;
        case HEAT:
            this->mode = climate::CLIMATE_MODE_HEAT;
            break;
        case FAN:
            this->mode = climate::CLIMATE_MODE_FAN_ONLY;
            break;
        case DRY:
            this->mode = climate::CLIMATE_MODE_DRY;
            break;
        default:
            ESP_LOGE(TAG, "Unknown power mode");
            return false;
        }
    } else {
        this->mode = climate::CLIMATE_MODE_OFF;
    }

    // Temperature
    this->target_temperature = temperature;

    // Fan speed
    switch (fanSpeed) {
    case FAN1:
        this->fan_mode = climate::CLIMATE_FAN_AUTO;
        break;
    case FAN2: // Only to support remote feedback
        this->fan_mode = climate::CLIMATE_FAN_LOW;
        break;
    case FAN3:
        this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
        break;
    case FAN4:
        this->fan_mode = climate::CLIMATE_FAN_HIGH;
        break;
    default:
        this->fan_mode = climate::CLIMATE_FAN_AUTO;
        break;
    }

    ESP_LOGD(TAG, "Finish it");

    this->publish_state();
    return true;
}

void MitsubishiSEZClimate::transmit_state()
{
    // Thanks to: https://github.com/ToniA/arduino-heatpumpir/blob/489f6eaa434096aa82ef8195055b4b7d99a2394e/MitsubishiSEZKDXXHeatpumpIR.cpp#L77
    uint8_t bytes[MSG_LENGTH] = { 0x23, 0xCB, 0x26, 0x21, 0x00, 0x40, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    // ----------------------
    // Initial values
    // ----------------------

    auto operatingMode = COOL;
    auto powerMode = OFF;
    auto temperature = 19;
    auto fanSpeed = FAN1;

    // ----------------------
    // Assign the values
    // ----------------------

    // Power and operating mode
    switch (this->mode) {
    case climate::CLIMATE_MODE_COOL:
        operatingMode = COOL;
        break;
    case climate::CLIMATE_MODE_HEAT:
        operatingMode = HEAT;
        break;
    case climate::CLIMATE_MODE_AUTO:
        operatingMode = AUTO;
        break;
    case climate::CLIMATE_MODE_FAN_ONLY:
        operatingMode = FAN;
        break;
    case climate::CLIMATE_MODE_DRY:
        operatingMode = DRY;
        break;
    case climate::CLIMATE_MODE_OFF:
    default:
        powerMode = OFF;
        break;
    }

    // Temperature
    if (this->target_temperature >= TEMP_MIN && this->target_temperature <= TEMP_MAX)
        temperature = this->target_temperature;

    // Fan speed
    switch (this->fan_mode.value()) {
    case climate::CLIMATE_FAN_LOW:
        fanSpeed = FAN2;
        break;
    case climate::CLIMATE_FAN_MEDIUM:
        fanSpeed = FAN3;
        break;
    case climate::CLIMATE_FAN_HIGH:
        fanSpeed = FAN4;
        break;
    case climate::CLIMATE_FAN_AUTO:
    default:
        fanSpeed = FAN1;
        break;
    }

    // ----------------------
    // Assign the bytes
    // ----------------------

    // Power state + operating mode
    bytes[5] = powerMode;

    // Temperature
    bytes[6] = ((temperature - 16) << 4) | operatingMode;

    // Fan speed
    bytes[7] = fanSpeed;

    // There is no real checksum, but some bytes are inverted
    bytes[11] = ~bytes[5];
    bytes[12] = ~bytes[6];
    bytes[13] = ~bytes[7];
    bytes[14] = ~bytes[8];
    bytes[15] = ~bytes[9];
    bytes[16] = ~bytes[10];

    // ESP_LOGD(TAG, "Sending MHI target temp: %.1f state: %02X mode: %02X temp: %02X", this->target_temperature, bytes[5], bytes[6], bytes[7]);

    ESP_LOGD(TAG,
        "Sent bytes 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X",
        bytes[0], bytes[1], bytes[2], bytes[3],
        bytes[4], bytes[5], bytes[6], bytes[7],
        bytes[8], bytes[9], bytes[10], bytes[11],
        bytes[12], bytes[13], bytes[14], bytes[15],
        bytes[16], bytes[17], bytes[18]);

    auto transmit = this->transmitter_->transmit();
    auto data = transmit.get_data();

    data->set_carrier_frequency(38000);

    // Header
    data->mark(HEADER_MARK);
    data->space(HEADER_SPACE);

    // Data
    for (uint8_t i : bytes)
        for (uint8_t j = 0; j < 8; j++) {
            data->mark(BIT_MARK);
            bool bit = i & (1 << j);
            data->space(bit ? ONE_SPACE : ZERO_SPACE);
        }
    data->mark(BIT_MARK);
    data->space(0);

    transmit.perform();
}
} // namespace mhi
} // namespace esphome
