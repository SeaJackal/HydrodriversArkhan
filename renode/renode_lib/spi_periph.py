import os
import re

from renode_lib.periph import Periph

DATA_RECEIVED_PATTERN = re.compile(r"Data received: 0x([0-9A-Fa-f]{1,2})")


class SpiPeriph(Periph):
    def __init__(self, bus: str, name: str):
        self.bus = bus
        self.name = name
        self._log_position = 0

    def init(self, env):
        env._execute(
            f'machine LoadPlatformDescriptionFromString "{self.name}: Mocks.DummySPISlave @ {self.bus}"'
        )
        env._execute(f"logLevel -1 sysbus.{self.bus}.{self.name}")
        dummy = getattr(getattr(env.machine.sysbus, self.bus), self.name)
        self.dummy = dummy
        self.env = env
        self._log_file_path = env.log_file_path
        return self.name, self

    def enqueue_response(self, response: bytes) -> None:
        for byte in response:
            self.dummy.EnqueueValue(byte)

    def get_received_data(self) -> bytes:
        if not os.path.exists(self._log_file_path):
            return bytes()
        self.env.emulation.CurrentLogger.Flush()

        with open(self._log_file_path, "r", encoding="utf-8", errors="replace") as log_file:
            log_file.seek(self._log_position)
            log_data = log_file.read()
            self._log_position = log_file.tell()

        return bytes(
            int(match.group(1), 16) for match in DATA_RECEIVED_PATTERN.finditer(log_data)
        )
