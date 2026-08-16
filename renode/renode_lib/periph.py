from abc import ABCMeta, abstractmethod, abstractproperty

class Periph():
    __metaclass__ = ABCMeta

    @abstractmethod
    def init(self, env) -> None:
        pass
