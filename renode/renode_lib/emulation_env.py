import logging

from pyrenode3.wrappers import Emulation, Monitor
from renode_lib.periph import Periph

MACHINE_NAME = "TEST_MACHINE"
LOG_FILE = "/tmp/renode.log"

class EmulationEnv:
    def __init__(self, elf_path: str, repl_path: str, peripherals: list[Periph] = []):
        self.logger = logging.getLogger(__name__)
        self.logger.info("Creating Renode emulation for %s", MACHINE_NAME)
        self.logger.info("Using ELF: %s", elf_path)
        self.logger.info("Using platform: %s", repl_path)

        self.emulation = Emulation()
        self.monitor = Monitor()
        self.machine = self.emulation.add_mach(MACHINE_NAME)

        self.machine.load_repl(repl_path)
        self._execute("cpu PerformanceInMips 200")
        self._execute('emulation SetGlobalQuantum "0.000001"')
        self.machine.load_elf(elf_path)
        self._execute("sysbus.cpu VectorTableOffset 0x08000000")
        self._execute("machine StartGdbServer 3333")
        self.log_file_path = LOG_FILE
        self._execute(f"logFile @{LOG_FILE}")

        for peripheral in peripherals:
            result = peripheral.init(self)
            setattr(self, result[0], result[1])
            

    def _execute(self, command: str) -> str:
        self.logger.debug("Renode command: %s", command)
        output, error = self.monitor.execute(command)
        if error:
            raise RuntimeError(f"Renode command failed: {command}\n{error}")
        return output