# 
APP-build := msnet

# 头文件路径
INCLUDES += -I ./  -I ./3rd
INCLUDES += -I ./3rd/comm-inc/include/common
INCLUDES += -I ./3rd/asio-1.12.2  
INCLUDES += -I ./3rd/x2struct-1.1 
INCLUDES += -I ./3rd/faac
INCLUDES += -I ./3rd/librtmp  
INCLUDES += -I ./utils

LDFLAGS += ./3rd/librtmp/librtmp.a ./3rd/faac/libfaac/.libs/libfaac.a

# LDFLAGS += -L/usr/local/lib -lavutil -lavformat -lavcodec

ifeq (y, $(USE_SRS_LIBRTMP))
CXXFLAGS += -DSRS_LIBRTMP
DIRS += srs
endif
# 编译选项
CXXFLAGS += $(INCLUDES) -DASIO_STANDALONE -DASIO_HAS_STD_CHRONO
#CXXFLAGS += -DUSE_PUB_RTMP
CXXFLAGS += -DUSE_FLV_SRV

# 生成可执行程序链接库
LDFLAGS += -ldl -lrt -pthread -Wl,-rpath=./

# 源文件
DIRS += utils g7xx flv
SRCS += Publisher.cpp \
		TcpBusiness.cpp \
		Conf.cpp \
		TcpConn.cpp \
		TcpConnManager.cpp \
		rtmp/RtmpWriter.cpp \
		Main.cpp \

-include scripts/compile.mk