# import logging
from esphome import pins
import esphome.codegen as cg
from esphome.components import climate, spi
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.cpp_helpers import gpio_pin_expression

CODEOWNERS = ["oliv3r"]

DEPENDENCIES = ["spi"]

CONF_DEBUG_SWITCH = "debug_switch"
CONF_MHI_AC_CTRL_ID = "mhi_ac_ctrl_id"
CONF_CLOCK_MONITOR_PIN = "clock_monitor_pin"
CONF_CS_SYNC_PIN = "cs_sync_pin"

mhiacctrl_ns = cg.esphome_ns.namespace("mhi_ac_ctrl")
MHIACCtrlComponent = mhiacctrl_ns.class_("MHIACCtrl", cg.Component, climate.Climate)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(MHIACCtrlComponent),
        cv.Required(CONF_CLOCK_MONITOR_PIN): pins.gpio_input_pin_schema,
        cv.Required(CONF_CS_SYNC_PIN): pins.gpio_output_pin_schema,
    }
).extend(
    spi.spi_device_schema(
        cs_pin_required=False,
        default_bit_order="lsb_first",
        default_mode=3,
        default_role="slave",
    )
)

FINAL_VALIDATE_SCHEMA = spi.final_validate_device_schema(
    CONF_MHI_AC_CTRL_ID, require_mosi=True, require_miso=True
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await spi.register_spi_device(var, config)

    clk_mon_pin = await gpio_pin_expression(config[CONF_CLOCK_MONITOR_PIN])
    cg.add(var.set_pin_clk_mon(clk_mon_pin))

    cs_sync_pin = await gpio_pin_expression(config[CONF_CS_SYNC_PIN])
    cg.add(var.set_pin_cs_sync(cs_sync_pin))
