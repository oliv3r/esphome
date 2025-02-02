import esphome.codegen as cg
from esphome.components import climate
import esphome.config_validation as cv

# from esphome.components.climate import (
#    ClimateMode,
#    ClimatePreset,
#    ClimateSwingMode,
#    CONF_CURRENT_TEMPERATURE,
#    CONF_SUPPORTS_DRY,
# )
from . import CONF_MHI_AC_CTRL_ID, MHIACCtrlComponent

PROTOCOL_MIN_TEMPERATURE = 18.0
PROTOCOL_MAX_TEMPERATURE = 30.0
PROTOCOL_TARGET_TEMPERATURE_STEP = 1.0
PROTOCOL_CURRENT_TEMPERATURE_STEP = 0.5

CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend(
    {
        cv.GenerateID(CONF_MHI_AC_CTRL_ID): cv.use_id(MHIACCtrlComponent),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    if clima := await cg.get_variable(config[CONF_MHI_AC_CTRL_ID]):
        cg.add_build_flag("-DMHI_USE_MOVING_AVERAGE_FILTER_TEMPERATURE_ROOM")
        await climate.register_climate(clima, config)
