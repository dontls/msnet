#ifndef CONF_H
#define CONF_H

#include <iostream>

typedef struct NginxConifg {
  std::string ip;
  std::string stat_port;
} NginxConifg_t;

typedef struct DevConfig {
  int max_num;
  int auto_close_time;
  int bytes_out;
} DevConfig_t;

typedef struct ServerConfig {
  int port;
} ServerConfig_t;

typedef struct XmlConfig {
  NginxConifg_t nginx;
  DevConfig_t dev;
  ServerConfig_t server;
} XmlConfig_t;

bool XmlConfigInit(const char* xmlPath);

struct XmlConfig* GetXmlConfig();

std::string GetRtmpBaseUrl();

#endif