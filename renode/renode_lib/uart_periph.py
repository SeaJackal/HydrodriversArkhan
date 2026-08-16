from renode_lib.periph import Periph

class UartPeriph(Periph):
    def __init__(self, name: str):
        self.name = name

    def init(self, env) -> None:
        env._execute(f'emulation CreateUartPtyTerminal "term_{self.name}" "/tmp/{self.name}" true')
        env._execute(f'connector Connect sysbus.{self.name} term_{self.name}')
        return "uart", getattr(env.machine.sysbus, self.name)