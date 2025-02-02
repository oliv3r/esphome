import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import (
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_THERMOMETER,
)

from . import CONF_MHI_AC_CTRL_ID, MHIACCtrlComponent, mhiacctrl_ns

ICON_BUG = "mdi:bug"
ICON_CONNECTION = "mdi:connection"
ICON_LOOP = "mdi:electric-switch"

MHIACLoopEnable = mhiacctrl_ns.class_("MHIACLoopEnable", cg.Component, switch.Switch)
MHIACDebugMode = mhiacctrl_ns.class_("MHIACDebugMode", cg.Component, switch.Switch)
MHIACInternalTempSensor = mhiacctrl_ns.class_(
    "MHIACInternalTempSensor", cg.Component, switch.Switch
)

CONF_DEBUG_MODE = "debug_mode"
CONF_LOOP_ENABLE = "loop_enable"
CONF_INTERNAL_TEMPERATURE_SENSOR = "internal_temperature_sensor"

SW_TYPES = {
    CONF_DEBUG_MODE: switch.switch_schema(
        class_=MHIACDebugMode,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        icon=ICON_BUG,
    ),
    CONF_LOOP_ENABLE: switch.switch_schema(
        class_=MHIACLoopEnable,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        icon=ICON_LOOP,
    ),
    CONF_INTERNAL_TEMPERATURE_SENSOR: switch.switch_schema(
        class_=MHIACInternalTempSensor,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        icon=ICON_THERMOMETER,
    ),
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MHI_AC_CTRL_ID): cv.use_id(MHIACCtrlComponent),
    }
).extend({cv.Optional(type_): schema for type_, schema in SW_TYPES.items()})


async def to_code(config):
    for type_ in SW_TYPES:
        if conf := config.get(type_):
            sw = await switch.new_switch(conf)
            await cg.register_parented(sw, config[CONF_MHI_AC_CTRL_ID])
            await cg.register_component(sw, config)
