# 
APP-build := msnet

# 头文件路径
INCLUDES += -I ./
INCLUDES += -I ./inc
INCLUDES += -I ./asio-1.12.2  
INCLUDES += -I ./faac/include
INCLUDES += -I ./utils
INCLUDES += -I ./src

LDFLAGS += ./librtmp/librtmp.a ./faac/libfaac/.libs/libfaac.a

# LDFLAGS += -L/usr/local/lib -lavutil -lavformat -lavcodec

ifeq (y, $(USE_SRS_LIBRTMP))
CXXFLAGS += -DSRS_LIBRTMP
DIRS += srs
endif
# 编译选项
CXXFLAGS += $(INCLUDES)
CXXFLAGS += -DUSE_FLV_SRV

# 生成可执行程序链接库
LDFLAGS += -ldl -lrt -pthread -Wl,-rpath=./

# 源文件
DIRS += src/g7xx src/flv src/rtmp src 
SRCS += Main.cpp

-include scripts/compile.mk