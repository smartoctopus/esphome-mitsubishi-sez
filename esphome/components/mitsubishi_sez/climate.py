import esphome.codegen as cg
from esphome.components import climate_ir

AUTO_LOAD = ['climate_ir']

mitsubishi_sez_ns = cg.esphome_ns.namespace('mitsubishi_sez')
MitsubishiSEZClimate = mitsubishi_sez_ns.class_('MitsubishiSEZClimate', climate_ir.ClimateIR)

CONFIG_SCHEMA = climate_ir.climate_ir_with_receiver_schema(MitsubishiSEZClimate)


async def to_code(config):
    await climate_ir.new_climate_ir(config)
