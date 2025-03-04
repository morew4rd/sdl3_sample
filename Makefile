default-task: me-dev

config-dev:
	cmake -DCMAKE_BUILD_TYPE=Release -S . -B _b

build-dev:
	cmake --build _b

run-dev:
	./_b/sdl-min.app/Contents/MacOS/sdl-min

config-web:
	emcmake cmake -DCMAKE_BUILD_TYPE=Release -S . -B _bw

build-web:
	cmake --build _bw

run-web:
	python -m http.server -d _bw 8000

me-dev: build-dev run-dev

me-web: build-web run-web