COMMON_INCLUDE_DIRS += $(rootdir)/$(MODULE)/include     \
                       $(incdir)/libonplatform            \
                       $(incdir)/libonmsgagent            \
                       $(incdir)/libonevent               \
                       $(incdir)/libontimer               \
                       $(incdir)/communication          \
                       $(incdir)/libonconfig              \
                       $(incdir)/penetrate

COMMON_SRC_FILES := $(rootdir)/$(MODULE)/src/on_core.c     \
                    $(rootdir)/$(MODULE)/src/on_nat.c      \
                    $(rootdir)/$(MODULE)/src/on_udp.c      \
                    $(rootdir)/$(MODULE)/src/on_msg.c

COMMON_INST_HEADER_DIRS += $(rootdir)/$(MODULE)/include
