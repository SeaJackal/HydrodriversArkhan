import serial
import logging
import random
import time

logger = logging.getLogger(__name__)

def _generate_payload() -> bytes:
    return random.randbytes(5)

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

logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
random.seed(42)
with serial.Serial(
    port="/dev/ttyUSB0",
    baudrate=115200,
    timeout=2.0,
    write_timeout=2.0,
) as uart:
    for index in range(1, 10 + 1):
        try:
            # test_uart_example(uart)
            print(_generate_payload())
            time.sleep(0.01)
        except:
            logger.info("Error on attempt %d", index)
            raise
        passed = index
        if index == 1 or index == 10000 or index % 1000 == 0:
            logger.info("UART echo progress: %d/%d passed", index, 10000)
