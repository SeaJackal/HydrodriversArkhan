import os
import random
import threading
import time
import logging
import serial
import sys

from pyrenode3.wrappers import Emulation, Monitor
from renode_lib.emulation_env import EmulationEnv
from renode_lib.uart_periph import UartPeriph

UART_DEVICE = "usart3"
UART_PATH = f"/tmp/{UART_DEVICE}"
PAYLOAD_LENGTH = 5
PAYLOAD_SEED = 42
DEFAULT_ITERATIONS = 10000
UART_BAUDRATE = 115200
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

def _generate_payload() -> bytes:
    return random.randbytes(PAYLOAD_LENGTH)


def test_uart_example(uart: serial.Serial) -> None:
    payload = _generate_payload()
    logger.debug("Sending UART payload: %r", payload)

    uart.reset_input_buffer()
    uart.reset_output_buffer()
    sent = uart.write(payload)
    uart.flush()
    if sent != len(payload):
        raise TimeoutError(f"Expected to send {len(payload)} bytes, sent {sent}")
    received = uart.read(len(payload))
    logger.debug("Received UART payload: %r", received)

    if len(received) != len(payload):
        raise TimeoutError(
            f"Expected {len(payload)} bytes, received {len(received)}: {received!r}"
        )

    if received != payload:
        raise AssertionError(f"Expected {payload!r}, received {received!r}")

def run_uart_example(iterations: int = DEFAULT_ITERATIONS) -> None:
    repl_path = sys.argv[1]
    elf_path = sys.argv[2]
    logger.info("Starting UART pyrenode3 test with %d iterations", iterations)
    uart_emulation = EmulationEnv(elf_path, repl_path, [UartPeriph(UART_DEVICE)])
    logger.info("Starting Renode emulation")
    uart_emulation.emulation.StartAll()
    random.seed(PAYLOAD_SEED)

    _wait_for_uart_device(UART_PATH)

    try:
        passed = 0
        progress_interval = max(1, iterations // 10)
        with serial.Serial(
            port=UART_PATH,
            baudrate=UART_BAUDRATE,
            timeout=2.0,
            write_timeout=2.0,
        ) as uart:
            logger.info("Opened UART serial port %s at %d baud", UART_PATH, UART_BAUDRATE)
            for index in range(1, iterations + 1):
                try:
                    test_uart_example(uart)
                    time.sleep(0.01)
                except:
                    logger.info("Error on attempt %d", index)
                    raise
                passed = index
                if index == 1 or index == iterations or index % progress_interval == 0:
                    logger.info("UART echo progress: %d/%d passed", index, iterations)
        logger.info("UART pyrenode3 test passed: %d/%d iterations", passed, iterations)
    except Exception:
        logger.exception("UART pyrenode3 test failed")
        raise
    finally:
        logger.info("Pausing Renode emulation")
        uart_emulation.emulation.PauseAll()


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
    run_uart_example()

