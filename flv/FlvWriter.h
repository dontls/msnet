#ifndef FLVWRITER_H
#define FLVWRITER_H

#include "FlvConn.h"
#include "FlvMuxer.h"
#include "ThMutex.h"
#include <map>
#include <set>
#include <string>

class FlvWriter {
private:
    CommMutex             _mtx;
    std::set<FlvConn_Ptr> _flvConns;
    FlvMuxer              _flvMuxer;
    std::string           _aacSpec;  // 记录aac spec
public:
    FlvWriter(/* args */);
    ~FlvWriter();

public:
    bool addFlvWriterConn(FlvConn_Ptr ptr);
    bool delFlvWriterConn(FlvConn_Ptr ptr);
    bool clearFlvWriterConn();

    // tagType audio0x08/video0x09
    void setFlvPacket(uint8_t tagType, std::string flvTag, unsigned long long apts, uint32_t vTagType = 0);
    void setSpecificConfig(std::string aacSpec);
};

typedef std::shared_ptr<FlvWriter> FlvWriter_Ptr;

class FlvWriterManager {
    std::map<std::string, FlvWriter_Ptr> _flvWriterManager;
    CommMutex                            _mtx;

public:
    static FlvWriterManager* ins();

    FlvWriter_Ptr flvWriter(std::string devID);
    void          delFlvWriter(std::string devID);
};

#endif