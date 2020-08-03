# Compilation tools
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar
LD = $(CROSS_COMPILE)ld
STRIP = $(CROSS_COMPILE)strip
NM = $(CROSS_COMPILE)nm
# export CC CXX AR LD NM STRIP 

Q := @
ECHO := @echo
TOPDIR := $(CURDIR)/..
export Q ECHO TOPDIR
#-------------------------------------------------------------------------------

build ?= debug

ifeq ($(build), debug)
CFLAGS_OPT +=-Wall -g
else
CFLAGS_OPT +=-DNDEBUG -Os 
endif

#-------------------------------------------------------------------------------
# 这里定义通用的编译参数，不同项目在对应Makefile中配置
# c 编译参数
CFLAGS += -fPIC -Wdeprecated-declarations -Wunused-variable $(CFLAGS_OPT)
# c++ 编译参数
CXXFLAGS += $(CFLAGS) -std=c++11

# Makefile build
SRCS += $(foreach dir,$(DIRS),$(wildcard $(dir)/*.cpp))
OBJS := $(patsubst %.cpp, %.o, $(filter %.cpp, $(SRCS)))
SO := $(findstring .so, $(LIB-build))

PHONY := all 
all:: $(LIB-build) $(APP-build)
	$(ECHO) -e "\033[36mDone $^\033[0m"

$(LIB-build):: $(OBJS)
ifneq (, $(SO))
	$(Q)$(CXX) -shared -fPIC -o $@ $^ $(LDFLAGS)
	$(STRIP) $@
else
	$(Q)$(AR) -rcs $@ $^
endif
	 
$(APP-build):: $(OBJS)
	$(Q)$(CXX) -o $@ $^ $(LDFLAGS)
	$(Q)$(STRIP) $@

PHONY += clean
clean: 
	$(Q)rm -rf $(OBJS) $(APP-build) $(LIB-build)
	
PHONY += distclean
distclean: clean
	$(Q)rm -f $(shell find $(CURDIR) -name "*.d")

.PHONY: $(PHONY)

# 
%.d: %.cpp
	@$(CXX) $(CXXFLAGS) -MM $< -MT $(basename $@).o -o $(basename $@).d

%.o: %.cpp
	@echo "CXX " $@;
	@$(CXX) $(CXXFLAGS) -c $< -o $@

%.d: %.c
	@$(CC) $(CFLAGS) -MM $< -MT $(basename $@).o -o $(basename $@).d

%.o: %.c
	@echo "CC " $@;
	@$(CC) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)
