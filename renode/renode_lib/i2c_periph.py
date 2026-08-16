from renode_lib.periph import Periph


class I2cPeriph(Periph):
    def __init__(self, bus: str, address: int, write_handler, read_handler, name: str = "dummy"):
        self.bus = bus
        self.address = address
        self.write_handler = write_handler
        self.read_handler = read_handler
        self.name = name

    def init(self, env):
        self.emulation = env.emulation
        env._execute(
            f'machine LoadPlatformDescriptionFromString "{self.name}: Mocks.DummyI2CSlave @ {self.bus} 0x{self.address:X}"'
        )
        dummy = getattr(getattr(env.machine.sysbus, self.bus), self.name)

        def on_data_written(data):
            self.write_handler(data)

        def on_data_requested(number_of_bytes: int):
            response = self.read_handler(number_of_bytes)
            if response:
                for byte in response:
                    dummy.EnqueueResponseByte(byte)

        dummy.ReadRequested += on_data_requested
        self._on_data_requested = on_data_requested
        dummy.DataReceived += on_data_written
        self._on_data_written = on_data_written
        return self.name, dummy
