CWD := $(shell pwd)

SERIAL_PORT = /dev/ttyUSB0
SERIAL_OPTS = -p $(SERIAL_PORT) -b 152000

ifeq ($(shell test -e $(SERIAL_PORT) && echo -n yes),yes)
	SERIAL_DEVICE := --device $(SERIAL_PORT)
else
	SERIAL_DEVICE :=
endif

DOCKER_EXE := docker

DOCKERFILE := docker/Dockerfile

PROJECT_DIR =
ENABLE_TTY =
MAKE_DOCKER = $(DOCKER_EXE) build - < $(DOCKERFILE) | tee /dev/stderr | grep "Successfully built" | cut -d ' ' -f 3
RUN = $(DOCKER_EXE) run \
  -v $(CWD):/project $(PROJECT_DIR) \
  $(SERIAL_DEVICE) \
  --user $(shell id -u):$(shell id -g) \
  --group-add dialout \
  --rm \
  -i $(ENABLE_TTY) `$(MAKE_DOCKER)`

build: PROJECT_DIR=-w /project/src
build:
	@$(RUN) idf.py build

flash: PROJECT_DIR=-w /project/src
flash:
	@$(RUN) idf.py $(SERIAL_OPTS) flash

monitor: PROJECT_DIR=-w /project/src
monitor: ENABLE_TTY=-t
monitor:
	@$(RUN) idf.py $(SERIAL_OPTS) monitor

shell: ENABLE_TTY=-t
shell:
	@$(RUN) bash

docker:
	@$(MAKE_DOCKER)

