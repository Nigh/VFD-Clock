

BUILD_DIR = build


.PHONY: clean default build rebuild assets

default: build

build:
# cmake -DPICO_COPY_TO_RAM=1 .. -G Ninja -B$(BUILD_DIR) -S.
	cmake .. -G Ninja -B$(BUILD_DIR) -S.
	ninja -C $(BUILD_DIR)

clean:
	@if [ -d "./$(BUILD_DIR)" ]; then rm ./$(BUILD_DIR) -r; fi

rebuild: clean build

format:
	bash format.sh

assets:
	./assets/p2a.exe -w -in ./assets/ui -out ./assets 
