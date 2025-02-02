import esphome.codegen as cg
from esphome.components import climate
import esphome.config_validation as cv

# from esphome.components.climate import (
#    ClimateMode,
#    ClimatePreset,
#    ClimateSwingMode,
#    CONF_CURRENT_TEMPERATURE,
# )
from esphome.core import coroutine

from . import CONF_MHI_AC_CTRL_ID, MHIACCtrlComponent

CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(MHIACCtrlComponent),
        cv.GenerateID(CONF_MHI_AC_CTRL_ID): cv.use_id(MHIACCtrlComponent),
    }
).extend(cv.COMPONENT_SCHEMA)


@coroutine
def to_code(config):
    parent = yield cg.get_variable(config[CONF_MHI_AC_CTRL_ID])
    yield climate.register_climate(parent, config)
