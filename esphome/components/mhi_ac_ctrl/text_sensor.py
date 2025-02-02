import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_DIAGNOSTIC, ICON_CHIP

from . import CONF_MHI_AC_CTRL_ID, MHIACCtrlComponent, mhiacctrl_ns

CONF_FRAME_SIZE = "frame_size"

MHIACFrameSize = mhiacctrl_ns.class_(
    "MHIACFrameSize", cg.Component, text_sensor.TextSensor
)

TEXT_SENSOR_TYPES = {
    CONF_FRAME_SIZE: text_sensor.text_sensor_schema(
        class_=MHIACFrameSize,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        icon=ICON_CHIP,
    ),
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MHI_AC_CTRL_ID): cv.use_id(MHIACCtrlComponent),
    }
).extend({cv.Optional(type_): schema for type_, schema in TEXT_SENSOR_TYPES.items()})


async def to_code(config):
    for type_ in TEXT_SENSOR_TYPES:
        if conf := config.get(type_):
            sens = await text_sensor.new_text_sensor(conf)
            await cg.register_parented(sens, config[CONF_MHI_AC_CTRL_ID])
            await cg.register_component(sens, config)
