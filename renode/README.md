# Руководство по работе с Renode

## Установка Renode

**Официальная документация:** [Инструкция по установке Renode](https://github.com/renode/renode/blob/master/README.md#installation)

### 1. Запускаем renode
renode -e 'set bin_path @TARGET.elf' \
       -e 'set renode_core_path @third_party/renode' \
       -e 'i @START_SCRIPT.resc'
       
TARGET Относительный путь до .elf файла
START_SCRIPT относитеьный путь до .resc файлика

Можно запускать через task в VScode
Потом запускаем отладку и газ 