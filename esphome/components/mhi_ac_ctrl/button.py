import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_DIAGNOSTIC

from . import CONF_MHI_AC_CTRL_ID, MHIACCtrlComponent, mhiacctrl_ns

CONF_FRAME_ERRORS_RESET = "frame_errors_reset"

ICON_ALERT = "mdi:alert-outline"

MHIACFrameErrorsReset = mhiacctrl_ns.class_(
    "MHIACFrameErrorsReset", cg.Component, button.Button
)

BUTTON_TYPES = {
    CONF_FRAME_ERRORS_RESET: button.button_schema(
        class_=MHIACFrameErrorsReset,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        icon=ICON_ALERT,
    ),
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MHI_AC_CTRL_ID): cv.use_id(MHIACCtrlComponent),
    }
).extend({cv.Optional(type_): schema for type_, schema in BUTTON_TYPES.items()})


async def to_code(config):
    for type_ in BUTTON_TYPES:
        if conf := config.get(type_):
            but = await button.new_button(conf)
            await cg.register_parented(but, config[CONF_MHI_AC_CTRL_ID])
            await cg.register_component(but, config)
