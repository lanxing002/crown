import crown
from enum import Enum


class Keyboard(Enum):
    w = crown.keyboard.button_id('w')
    a = crown.keyboard.button_id('a')
    s = crown.keyboard.button_id('s')
    d = crown.keyboard.button_id('d')
