import logging
import sys

from pyrenode3.wrappers import emulation

from renode_lib.emulation_env import EmulationEnv
from Antmicro.Renode.Time import TimeInterval

PWM_LED = "pwm_led"
SAMPLE_STEP_MS = 5
SAMPLE_WINDOW_MS = 300
FIRST_DUTY_MIN = 0.55
SECOND_DUTY_MAX = 0.45
logger = logging.getLogger(__name__)


class TimerPwmPeriph:
    def init(self, env):
        env._execute(
            'machine LoadPlatformDescriptionFromString "'
            f'{PWM_LED}: Miscellaneous.LED @ sysbus\n'
            'timer5:\n'
            f'    0 -> {PWM_LED}@0"'
        )
        return PWM_LED, getattr(env.machine.sysbus, PWM_LED)


def tim_example_should_update_pwm_duty() -> None:
    repl_path = sys.argv[1]
    elf_path = sys.argv[2]
    logger.info("Starting TIM pyrenode3 test")
    tim_emulation = EmulationEnv(elf_path, repl_path, [TimerPwmPeriph()])
    logger.info("Advancing Renode emulation")
    input("Press enter...")
    try:
        tim_emulation.emulation.RunFor(TimeInterval.FromMilliseconds(2000))

        for i in range(20):
            if not bool(tim_emulation.pwm_led.State):
                logger.debug(f"Found low state on {i*250} us")
                break
            tim_emulation.emulation.RunFor(TimeInterval.FromMicroseconds(250))
        else:
            raise TimeoutError(f"Synchronization error: no low state")
        
        for i in range(10000):
            if bool(tim_emulation.pwm_led.State):
                logger.debug(f"Found raise on {i} us")
                break
            tim_emulation.emulation.RunFor(TimeInterval.FromMicroseconds(1))
        else:
            raise TimeoutError(f"Synchronization error: no raise")

        for _ in range(7500):
            if not bool(tim_emulation.pwm_led.State):
                raise AssertionError(f"Expected updated high state")    
            tim_emulation.emulation.RunFor(TimeInterval.FromMicroseconds(1))
        for _ in range(2500):
            if bool(tim_emulation.pwm_led.State):
                raise AssertionError(f"Expected updated low state")    
            tim_emulation.emulation.RunFor(TimeInterval.FromMicroseconds(1))

        logger.info("TIM pyrenode3 test passed")

    except Exception:
        logger.exception("TIM pyrenode3 test failed")
        raise
    finally:
        logger.info("Pausing Renode emulation")
        tim_emulation.emulation.PauseAll()


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG, format="%(asctime)s %(levelname)s %(message)s")
    tim_example_should_update_pwm_duty()
