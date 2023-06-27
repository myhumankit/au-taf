all: final config.bin


OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
BUILD = ./picovoice

config.bin: config.json json2params.py
	python ./json2params.py

final: config.bin $(BUILD)
	python ./overwrite_section.py $(BUILD) config.bin final

