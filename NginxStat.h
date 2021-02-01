#ifndef NGINXSTAT_H
#define NGINXSTAT_H

struct StreamItem {
    std::string name;
    int         clis;
    int64_t     bytesOut;
};

#include "Asiohttp.h"
#include "Conf.h"
#include "TcpConnManager.h"
#include "rapidxml-1.13/rapidxml.hpp"
#include "rapidxml-1.13/rapidxml_print.hpp"

inline void doNginxStatAccess(XmlConfig* conf, const char* method, const char* page, const char* param)
{
    Asiohttp http;
    http.initUrl(conf->nginx.ip.c_str(), conf->nginx.stat_port.c_str());
    std::string reponse;
    if (!strcmp(method, "POST")) {
        http.post(page, param, reponse);
    } else if (!strcmp(method, "GET")) {
        http.get(page, param, reponse);
    } else {
        return;
    }
    // std::cout << reponse.c_str() << "\n";
    rapidxml::xml_document<> doc;
    doc.parse<0>(( char* )reponse.c_str());
    rapidxml::xml_node<>* root = doc.first_node();
    rapidxml::xml_node<>* streamNode =
        root->first_node("server")->first_node("application")->first_node("live")->first_node("stream");
    if (streamNode == NULL) {
        return;
    }
    auto TcpConnManager = TcpConnManager::ins();
    while (streamNode) {
        if (strcmp(streamNode->name(), "stream") == 0) {
            StreamItem st;
            st.name = streamNode->first_node("name")->value();
            st.bytesOut = atoi(streamNode->first_node("bytes_out")->value());
            st.clis = atoi(streamNode->first_node("nclients")->value());
            std::cout << "name " << st.name << " bytes_out " << st.bytesOut << " nclients " << st.clis << "\n";
            TcpConnManager->removeTcpConn(st.name, st.clis, conf->dev.auto_close_time);
            streamNode = streamNode->next_sibling();
        } else {
            break;
        }
    }
}

#endif