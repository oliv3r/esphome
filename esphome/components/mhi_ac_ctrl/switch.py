import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_CONFIG, ENTITY_CATEGORY_DIAGNOSTIC

from . import CONF_MHI_AC_CTRL_ID, MHIACCtrlComponent, mhiacctrl_ns

ICON_BUG = "mdi:bug"
ICON_CONNECTION = "mdi:connection"

MHIACActiveMode = mhiacctrl_ns.class_("MHIACActiveMode", cg.Component, switch.Switch)
MHIACDebugMode = mhiacctrl_ns.class_("MHIACDebugMode", cg.Component, switch.Switch)

CONF_ACTIVE_MODE = "active_mode"
CONF_DEBUG_MODE = "debug_mode"

SW_TYPES = {
    CONF_ACTIVE_MODE: switch.switch_schema(
        class_=MHIACActiveMode,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon=ICON_CONNECTION,
    ),
    CONF_DEBUG_MODE: switch.switch_schema(
        class_=MHIACDebugMode,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        icon=ICON_BUG,
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
