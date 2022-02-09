compile: 
	arduino-cli compile --fqbn esp32:esp32:esp32-gateway:Revision=RevF esp32-dante-test.ino

upload:
	arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32-gateway:Revision=RevF esp32-dante-test.ino

