import os
import logging
import sys

from pyrenode3.wrappers import Emulation, Monitor
from Antmicro.Renode.Time import TimeInterval
from renode_lib.emulation_env import EmulationEnv
from renode_lib.gpio_periph import GpioPeriph

logger = logging.getLogger(__name__)

def gpio_example_should_blink_led() -> None:
    repl_path = sys.argv[1]
    elf_path = sys.argv[2]
    logger.info("Starting GPIO pyrenode3 test")
    gpio_emulation = EmulationEnv(elf_path, repl_path, [GpioPeriph("led")])
    logger.info("Advancing Renode emulation")
    try:
        gpio_emulation.emulation.RunFor(TimeInterval.FromMilliseconds(1))
        if bool(gpio_emulation.led.State) != True:
            raise TimeoutError(f"Expected LED state {True}, current state is {bool(gpio_emulation.led.State)}")
        gpio_emulation.emulation.RunFor(TimeInterval.FromMilliseconds(1000))
        if bool(gpio_emulation.led.State) != False:
            raise TimeoutError(f"Expected LED state {False}, current state is {bool(gpio_emulation.led.State)}")
        logger.info("GPIO pyrenode3 test passed")
    except Exception:
        logger.exception("GPIO pyrenode3 test failed")
        raise
    finally:
        logger.info("Pausing Renode emulation")
        gpio_emulation.emulation.PauseAll()


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG, format="%(asctime)s %(levelname)s %(message)s")
    gpio_example_should_blink_led()
