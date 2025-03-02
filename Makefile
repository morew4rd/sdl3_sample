default-task: build-example run-example

config-example:
	cmake -DCMAKE_BUILD_TYPE=Release -S . -B _b

build-example:
	cmake --build _b

run-example:
	_b/sdl3-mini/app

config-example-web:
	emcmake cmake -DCMAKE_BUILD_TYPE=Release -S . -B _bw

build-example-web:
	cmake --build _bw

run-example-web:
	python -m http.server -d _bw/Release 8000