# import logging
from esphome import pins
import esphome.codegen as cg
from esphome.components import spi
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PIN_A, CONF_PIN_B, CONF_PIN_C, CONF_PIN_D
from esphome.cpp_helpers import gpio_pin_expression

CODEOWNERS = ["oliv3r"]

DEPENDENCIES = ["spi"]

CONF_DEBUG_SWITCH = "debug_switch"
CONF_MHI_AC_CTRL_ID = "mhi_ac_ctrl_id"
CONF_CLOCK_MONITOR_PIN = "clock_monitor_pin"
CONF_CS_SYNC_PIN = "cs_sync_pin"

mhiacctrl_ns = cg.esphome_ns.namespace("mhi_ac_ctrl")
MHIACCtrlComponent = mhiacctrl_ns.class_("MHIACCtrl", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(MHIACCtrlComponent),
        cv.Required(CONF_CLOCK_MONITOR_PIN): pins.gpio_input_pin_schema,
        cv.Required(CONF_CS_SYNC_PIN): pins.gpio_output_pin_schema,
        cv.Optional(CONF_PIN_A): pins.gpio_input_pin_schema,
        cv.Optional(CONF_PIN_B): pins.gpio_input_pin_schema,
        cv.Optional(CONF_PIN_C): pins.gpio_output_pin_schema,
        cv.Optional(CONF_PIN_D): pins.gpio_output_pin_schema,
    }
).extend(spi.spi_device_schema(cs_pin_required=False))

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

    if CONF_PIN_A in config:
        pin = await gpio_pin_expression(config[CONF_PIN_A])
        cg.add(var.set_pin_a(pin))

    if CONF_PIN_B in config:
        pin = await gpio_pin_expression(config[CONF_PIN_B])
        cg.add(var.set_pin_b(pin))

    if CONF_PIN_C in config:
        pin = await gpio_pin_expression(config[CONF_PIN_C])
        cg.add(var.set_pin_c(pin))

    if CONF_PIN_D in config:
        pin = await gpio_pin_expression(config[CONF_PIN_D])
        cg.add(var.set_pin_d(pin))
