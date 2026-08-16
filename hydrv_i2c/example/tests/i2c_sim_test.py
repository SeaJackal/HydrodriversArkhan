import os
import time
import logging
import serial
import sys
import threading

from renode_lib.emulation_env import EmulationEnv
from renode_lib.i2c_periph import I2cPeriph
from renode_lib.uart_periph import UartPeriph
from Antmicro.Renode.Time import TimeInterval

I2C_BUS = "i2c1"
I2C_ADDRESS = 0x36
ANGLE_REGISTER = 0x0C
READ_LENGTH = 2
ANGLE_VALUES = [0x1234 + index for index in range(10)]
UART_DEVICE = "usart3"
UART_PATH = f"/tmp/{UART_DEVICE}"
UART_BAUDRATE = 115200
UART_TIMEOUT = 5.0
I2C_TIMEOUT = 5.0
LINE_END = b"\n\r"
RUN_AFTER_READ_MS = 10
logger = logging.getLogger(__name__)


def _wait_for_uart_device(path: str, timeout: float = 2.0) -> None:
    logger.info("Waiting for UART device: %s", path)
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if os.path.exists(path):
            logger.info("UART device is ready: %s", path)
            return
        time.sleep(0.01)
    raise TimeoutError(f"UART device was not created: {path}")


def _read_angle(uart: serial.Serial, angle: int):
    log_str = f"[I2CExample] [INFO] angle: {angle}\n\r".encode("ascii")
    result = uart.read(len(log_str))
    logger.debug("Got UART data: %r", result)
    if(result != log_str):
        raise AssertionError(f"Expected {log_str}, got {result}")


class AngleResponder:
    def __init__(self, angles: list[int]):
        self.angles = angles
        self.next_angle_index = 0
        self.read_requested = threading.Event()
        self.request_counter = 0

    def write(self, data):
        logger.debug("Got write request: %r", data)
        if ANGLE_REGISTER != data[0] and len(data) != 1:
            raise AssertionError(f"Expected write to {ANGLE_REGISTER}, got {data[0]}, length {len(data)}")

    def read(self, number_of_bytes: int):
        # if(number_of_bytes != READ_LENGTH):
        #     raise AssertionError(f"Expected read length {READ_LENGTH}, got {number_of_bytes}")
        logger.debug("Got read request: %d", number_of_bytes)
        self.request_counter = (self.request_counter + 1) % 2
        angle = self.angles[self.next_angle_index]
        if self.request_counter == 0:
            self.next_angle_index += 1
            self.next_angle_index %= len(self.angles)
            self.read_requested.set()
            return [angle & 0xFF]
        else:
            return [angle >> 8]

    def get_current_angle(self) -> int:
        return self.angles[self.next_angle_index]

    def wait_for_read(self) -> None:
        if not self.read_requested.wait(I2C_TIMEOUT):
            raise TimeoutError("Timed out waiting for I2C read request")
        self.read_requested.clear()
    


def i2c_example_should_read_angle() -> None:
    repl_path = sys.argv[1]
    elf_path = sys.argv[2]
    angle_responder = AngleResponder(ANGLE_VALUES)
    logger.info("Starting I2C pyrenode3 test")
    i2c_emulation = EmulationEnv(
        elf_path,
        repl_path,
        [
            I2cPeriph(I2C_BUS, I2C_ADDRESS, angle_responder.write, angle_responder.read),
            UartPeriph(UART_DEVICE),
        ],
    )

    _wait_for_uart_device(UART_PATH)

    try:
        with serial.Serial(
            port=UART_PATH,
            baudrate=UART_BAUDRATE,
            timeout=2,
            write_timeout=2.0,
        ) as uart:
            logger.info("Opened UART serial port %s at %d baud", UART_PATH, UART_BAUDRATE)
            uart.reset_input_buffer()
            uart.reset_output_buffer()
            logger.info("Starting Renode emulation")
            for expected_angle in ANGLE_VALUES:
                i2c_emulation.emulation.StartAll()
                angle_responder.wait_for_read()
                logger.debug("Continuing after read request")
                i2c_emulation.emulation.PauseAll()
                i2c_emulation.emulation.RunFor(TimeInterval.FromMilliseconds(RUN_AFTER_READ_MS))
                _read_angle(uart, expected_angle)

        logger.info("I2C pyrenode3 test passed")
    except Exception:
        logger.exception("I2C pyrenode3 test failed")
        raise
    finally:
        logger.info("Stopping Renode emulation")
        i2c_emulation.emulation.PauseAll()
        i2c_emulation.emulation.clear()


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
    i2c_example_should_read_angle()
