// -*- mode: C++;  tab-width: 2; c-basic-offset: 2;  -*-

#include <Adafruit_LSM9DS1.h>

void calibrateSensor(Adafruit_LSM9DS1 *lsm, sensors_vec_t *zeroAccel);
float calculateMagnitude(Adafruit_LSM9DS1 *lsm, sensors_vec_t *accelOut, sensors_vec_t *accelDeltaOut, const sensors_vec_t *zeroAccel);
