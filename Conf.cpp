#include "Conf.h"
#include "3rd/x2struct-1.1/thirdparty/rapidxml/rapidxml.hpp"
#include "3rd/x2struct-1.1/thirdparty/rapidxml/rapidxml_utils.hpp"
static struct XmlConfig xmlConf;

bool XmlConfigInit(const char* xmlPath)
{
    try {
        rapidxml::file<>         fdoc(xmlPath);
        rapidxml::xml_document<> doc;  // character type defaults to char
        doc.parse<0>(fdoc.data());     // 0 means default parse flags
        rapidxml::xml_node<>* root = doc.first_node();
        // nginx
        rapidxml::xml_node<>* nginxNode = root->first_node("nginx");
        xmlConf.nginx.ip = nginxNode->first_node("ip")->value();
        xmlConf.nginx.stat_port = nginxNode->first_node("stat_port")->value();

        // device
        rapidxml::xml_node<>* devNode = root->first_node("device");
        xmlConf.dev.max_num = atoi(devNode->first_node("max_num")->value());
        xmlConf.dev.auto_close_time = atoi(devNode->first_node("auto_close_time")->value());
        xmlConf.dev.bytes_out = atoi(devNode->first_node("bytes_out")->value());

        // device
        rapidxml::xml_node<>* serverNode = root->first_node("server");
        xmlConf.server.port = atoi(serverNode->first_node("port")->value());
        // doc.clear();
    }
    catch (...) {
        return false;
    }
    return true;
}

XmlConfig* GetXmlConfig()
{
    return &xmlConf;
}

std::string GetRtmpBaseUrl()
{
    std::string rtmpUrl = "rtmp://";
    return rtmpUrl.append(xmlConf.nginx.ip).append(":1935/live/");
}