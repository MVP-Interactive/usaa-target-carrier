// -*- mode: C++;  tab-width: 2; c-basic-offset: 2;  -*-

#include <Adafruit_LSM9DS1.h>

#include "sensor_calibration.h"

/// Calibrate the sensor by taking a number of readings and averaging them
void calibrateSensor(Adafruit_LSM9DS1 *lsm, sensors_vec_t *zeroAccel) {
  const int numReadings = 25;
  float sumX = 0.0, sumY = 0.0, sumZ = 0.0;
  sensors_event_t a, m, g, temp;

  for (int i = 0; i < numReadings; i++) {
    lsm->read();
    if (lsm->getEvent(&a, &m, &g, &temp)) {
      sumX += a.acceleration.x;
      sumY += a.acceleration.y;
      sumZ += a.acceleration.z;
    }
    delay(10); // small delay between readings
  }

  *zeroAccel = {
    sumX / numReadings,
    sumY / numReadings,
    sumZ / numReadings
  };

  Serial.printf("Calibration done - zeroAccel: (X: %.4f, Y: %.4f, Z: %.4f)\n", zeroAccel->x, zeroAccel->y, zeroAccel->z);
}

/// Calculate the magnitude of the acceleration vector
float calculateMagnitude(Adafruit_LSM9DS1 *lsm, sensors_vec_t *accelOut, sensors_vec_t *accelDeltaOut, const sensors_vec_t *zeroAccel) {
  sensors_event_t a, m, g, temp;

  lsm->read();
  if (lsm->getEvent(&a, &m, &g, &temp)) {
    float calibratedX = a.acceleration.x - zeroAccel->x;
    float calibratedY = a.acceleration.y - zeroAccel->y;
    float calibratedZ = a.acceleration.z - zeroAccel->z;

    *accelOut = a.acceleration;
    *accelDeltaOut = {
      calibratedX,
      calibratedY,
      calibratedZ
    };

    float magnitude_sq = 
      (calibratedX * calibratedX) + 
      (calibratedY * calibratedY) + 
      (calibratedZ * calibratedZ);
    return magnitude_sq;
  }
  return 0.0; // return 0 if the event was not successfully read
}
