from renode_lib.periph import Periph

class GpioPeriph(Periph):
    def __init__(self, name: str):
        self.name = name

    def init(self, env) -> None:
        return self.name, env.machine.sysbus.UserLED
