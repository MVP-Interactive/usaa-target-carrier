#ifndef USAA_API_H
#define USAA_API_H

class APIConfig {
 public:
  float threshold;
  int32_t hit_wait;
  int32_t hit_flash;

  bool threshold_is_set;
  bool hit_wait_is_set;
  bool hit_flash_is_set;
};

void APIUpdateUrls(uint8_t sensor_id);
void APIPostHit();
void APIPostStatus(uint8_t sensor_id);
APIConfig APIGetConfig();

#endif 
