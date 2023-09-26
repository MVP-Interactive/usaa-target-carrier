#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>    // not used in this demo but required!
#include <Adafruit_NeoPixel.h>  //  Library that provides NeoPixel functions

// i2c
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

void setupSensor() {
  // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_8G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_16G);

  // 2.) Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_8GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_12GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_16GAUSS);

  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_500DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_2000DPS);
}


void setup() {
  //pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  //delay(1000);

  while (!Serial) {
    delay(1); // will pause Zero, Leonardo, etc until serial console opens
  }

  Serial.println("\n Starting...\n");
  Serial.flush();


  Serial.println("LSM9DS1 data read demo");
  //  pinMode(PIN_I2C_POWER, INPUT);
  delay(1);
  //  bool polarity = digitalRead(PIN_I2C_POWER);
  //pinMode(PIN_I2C_POWER, OUTPUT);
  //digitalWrite(PIN_I2C_POWER, !polarity);

  Wire.begin (13, 16);  // init I2C on the respective pins

  // Try to initialise and warn if we couldn't detect the chip
  if (!lsm.begin()) {
    Serial.println("Oops ... unable to initialize the LSM9DS1. Check your wiring!");
    Serial.flush();
    while (1)
      ;
  }
  //Serial.println("Found LSM9DS1 9DOF");

  // helper to just set the default scaling we want, see above!
  setupSensor();
}

void loop() {
  //digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)

  lsm.read(); /* ask it to read in the data */

  /* Get a new sensor event */
  sensors_event_t a, m, g, temp;

  lsm.getEvent(&a, &m, &g, &temp);

  Serial.print("A_X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", ");
  Serial.print("A_Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", ");
  Serial.print("A_Z: ");
  Serial.print(a.acceleration.z);
  Serial.println("");

  // Serial.print("Mag X: "); Serial.print(m.magnetic.x);   Serial.print(" uT");
  // Serial.print("\tY: "); Serial.print(m.magnetic.y);     Serial.print(" uT");
  // Serial.print("\tZ: "); Serial.print(m.magnetic.z);     Serial.println(" uT");

  //  Serial.print("Gyro X: "); Serial.print(g.gyro.x);   Serial.print(" rad/s");
  //  Serial.print("\tY: "); Serial.print(g.gyro.y);      Serial.print(" rad/s");
  //  Serial.print("\tZ: "); Serial.print(g.gyro.z);      Serial.println(" rad/s");

  //Serial.println();

  int rc = 30, gc = 0, bc = 100;  //  Red, green and blue intensity to display

  if (a.acceleration.x > 0 || a.acceleration.y > 0) {
    rc = 200, gc = 0, bc = 0;
  }

  delay(50);
  //   digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)


  delay(50);
}