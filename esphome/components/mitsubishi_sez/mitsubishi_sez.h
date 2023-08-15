#pragma once

#include "esphome/components/climate_ir/climate_ir.h"

namespace esphome {
namespace mitsubishi_sez {

const uint8_t TEMP_MIN = 19; // Celsius
const uint8_t TEMP_MAX = 30; // Celsius

class MitsubishiSEZClimate : public climate_ir::ClimateIR {
public:
    MitsubishiSEZClimate()
        : climate_ir::ClimateIR(
            TEMP_MIN, TEMP_MAX, 1.0f, true, true,
            std::set<climate::ClimateFanMode> {
                climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW,
                climate::CLIMATE_FAN_MEDIUM, climate::CLIMATE_FAN_HIGH })
    {
    }

protected:
    void transmit_state() override;
    /// Handle received IR Buffer
    bool on_receive(remote_base::RemoteReceiveData data) override;
};
} // namespace mitsubishi_sez
} // namespace esphome
