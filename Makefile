compile: 
	arduino-cli compile --fqbn esp32:esp32:esp32-gateway:Revision=RevF esp32-dante-testino

upload:
	arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32-gateway:Revision=RevF esp32-dante-testino

