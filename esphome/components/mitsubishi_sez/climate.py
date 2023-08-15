import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate_ir
from esphome.const import CONF_ID

AUTO_LOAD = ['climate_ir']

mitsubishi_sez_ns = cg.esphome_ns.namespace('mitsubishi_sez')
MitsubishiSEZClimate = mitsubishi_sez_ns.class_('MitsubishiSEZClimate', climate_ir.ClimateIR)

CONFIG_SCHEMA = climate_ir.CLIMATE_IR_WITH_RECEIVER_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(MitsubishiSEZClimate),
})


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield climate_ir.register_climate_ir(var, config)
