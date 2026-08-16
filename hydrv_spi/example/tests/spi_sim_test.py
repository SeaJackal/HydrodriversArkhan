import logging
import sys

from renode_lib.emulation_env import EmulationEnv
from renode_lib.spi_periph import SpiPeriph
from Antmicro.Renode.Time import TimeInterval

SPI_BUS = "spi1"
SPI_VALUES = bytes([0x76, 0x00, 0x80 | 0x75, 0x00])
logger = logging.getLogger(__name__)

def spi_example_should_read_whoami() -> None:
    repl_path = sys.argv[1]
    elf_path = sys.argv[2]
    logger.info("Starting SPI pyrenode3 test")
    spi_emulation = None

    try:
        spi_emulation = EmulationEnv(
            elf_path,
            repl_path,
            [
                SpiPeriph(SPI_BUS, "test_spi")
            ],
        )
        logger.info("Starting Renode emulation")
        spi_emulation.emulation.RunFor(TimeInterval.FromMilliseconds(100))
        for index in range(10):
            received_data = bytes()
            received_data += spi_emulation.test_spi.get_received_data()
            if(received_data != SPI_VALUES):
                raise AssertionError(f"Expected {SPI_VALUES.hex()}, got {received_data.hex()}")
            if index != 9:
                spi_emulation.emulation.RunFor(TimeInterval.FromMilliseconds(500))
        logger.info("SPI pyrenode3 test passed")
    except Exception:
        logger.exception("SPI pyrenode3 test failed")
        raise
    finally:
        if spi_emulation is not None:
            logger.info("Stopping Renode emulation")
            spi_emulation.emulation.PauseAll()
            spi_emulation.emulation.clear()


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG, format="%(asctime)s %(levelname)s %(message)s")
    spi_example_should_read_whoami()
