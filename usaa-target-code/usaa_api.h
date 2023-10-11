// -*- mode: C++;  tab-width: 2; c-basic-offset: 2;  -*-

#ifndef USAA_API_H
#define USAA_API_H

class APIConfig {
public:
  float threshold;
  uint32_t hit_wait;
  uint32_t hit_flash;
  uint8_t white_level;
  uint32_t blink_interval;

  bool threshold_is_set;
  bool hit_wait_is_set;
  bool hit_flash_is_set;
  bool white_level_is_set;
  bool blink_interval_is_set;
};

void APIUpdateUrls(uint8_t sensor_id);
void APIPostHit();
void APIPostStatus(uint8_t sensor_id,  const String & rev);
APIConfig APIGetConfig();

#endif
