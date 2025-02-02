import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_DIAGNOSTIC, STATE_CLASS_TOTAL_INCREASING

from . import CONF_MHI_AC_CTRL_ID, MHIACCtrlComponent, mhiacctrl_ns

CONF_FRAME_ERRORS = "frame_errors"

ICON_ALERT = "mdi:alert-outline"

MHIACFrameErrors = mhiacctrl_ns.class_("MHIACFrameErrors", cg.Component, sensor.Sensor)

SENSOR_TYPES = {
    CONF_FRAME_ERRORS: sensor.sensor_schema(
        class_=MHIACFrameErrors,
        accuracy_decimals=0,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        icon=ICON_ALERT,
        state_class=STATE_CLASS_TOTAL_INCREASING,
    ),  # .extend(cv.polling_component_schema("60s")),
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MHI_AC_CTRL_ID): cv.use_id(MHIACCtrlComponent),
    }
).extend({cv.Optional(type_): schema for type_, schema in SENSOR_TYPES.items()})


async def to_code(config):
    for type_ in SENSOR_TYPES:
        if conf := config.get(type_):
            sens = await sensor.new_sensor(conf)
            await cg.register_parented(sens, config[CONF_MHI_AC_CTRL_ID])
            await cg.register_component(sens, config)
