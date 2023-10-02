# 9dof sensor/light package with PoE

## About
Connect an ESP32-POE-ISO to a lsm9ds1 sensor via StemmaQT and a WS2811 
light strand along with 12V power.

Because of the 12V power for the LEDs, the PoE on the ESP32-POE will not be used.

## Connections

Attach the Adafruit LSM9D1 Stemma power to the small 4 pin connector.

Connect the USB cable to the Olimex ESP32 board.

Connect the 10 pin ribbon cable to the Olimex ESP32 board.

Connect the LEDs to the 3 pin LED connector.

Connect the PoE extractor's ethernet out to the Olimex ESP32 board.

Connect the PoE extractor's 12v barrel to the 12v barrel on the board.

IF using debug/programming USB-C board, not that while you can physically plug the USB-C in other direction, only one direction will work due to a flay in the board.


## BOM
https://www.adafruit.com/product/4634
https://www.adafruit.com/product/4210
https://www.olimex.com/Products/IoT/ESP32/ESP32-POE-ISO/open-source-hardware

https://www.amazon.com/gp/product/B01AG923EU/

https://www.amazon.com/gp/product/B07L7QDZJW/

https://www.amazon.com/dp/B01DC0KIT2


