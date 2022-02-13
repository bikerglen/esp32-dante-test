compileg: 
	arduino-cli compile --fqbn esp32:esp32:esp32-gateway:Revision=RevF esp32-dante-test.ino

uploadg:
	arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32-gateway:Revision=RevF esp32-dante-test.ino

compilew: 
	arduino-cli compile --fqbn esp32:esp32:esp32 esp32-dante-test.ino

uploadw:
	arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 esp32-dante-test.ino

