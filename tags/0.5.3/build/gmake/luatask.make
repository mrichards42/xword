# GNU Make project makefile autogenerated by Premake
ifndef config
  config=release
endif

ifndef verbose
  SILENT = @
endif

ifndef CC
  CC = gcc
endif

ifndef CXX
  CXX = g++
endif

ifndef AR
  AR = ar
endif

ifeq ($(config),release)
  OBJDIR     = obj/Release/luatask
  TARGETDIR  = ../../bin/Release/scripts/libs
  TARGET     = $(TARGETDIR)/libtask.so
  DEFINES   += -DNDEBUG -DLUA_LIB -DLUA_USE_LINUX -DLUATASK_PTHREAD_STACK_SIZE=2097152/16 -DLUATASK_API=""
  INCLUDES  += -I../../lua/lua -I../../lua/lua/include
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -O2 -fPIC
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -s -shared -L../../bin/Release -L../../lib/Release
  LIBS      += -llua5.1 -ldl -lpthread
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += ../../bin/Release/liblua5.1.so
  LINKCMD    = $(CC) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),debug)
  OBJDIR     = obj/Debug/luatask
  TARGETDIR  = ../../bin/Debug/scripts/libs
  TARGET     = $(TARGETDIR)/libtask.so
  DEFINES   += -DDEBUG -D_DEBUG -DLUA_LIB -DLUA_USE_LINUX -DLUA_USE_APICHECK -DLUATASK_PTHREAD_STACK_SIZE=2097152/16 -DLUATASK_API=""
  INCLUDES  += -I../../lua/lua -I../../lua/lua/include
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g -fPIC
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -shared -L../../bin/Debug -L../../lib/Debug
  LIBS      += -llua5.1 -ldl -lpthread
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += ../../bin/Debug/liblua5.1.so
  LINKCMD    = $(CC) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

OBJECTS := \
	$(OBJDIR)/ltask.o \
	$(OBJDIR)/queue.o \
	$(OBJDIR)/syncos.o \

RESOURCES := \

SHELLTYPE := msdos
ifeq (,$(ComSpec)$(COMSPEC))
  SHELLTYPE := posix
endif
ifeq (/bin,$(findstring /bin,$(SHELL)))
  SHELLTYPE := posix
endif

.PHONY: clean prebuild prelink

all: $(TARGETDIR) $(OBJDIR) prebuild prelink $(TARGET)

$(TARGET): $(GCH) $(OBJECTS) $(LDDEPS) $(RESOURCES)
	@echo Linking luatask
	$(SILENT) $(LINKCMD)
	$(POSTBUILDCMDS)

$(TARGETDIR):
	@echo Creating $(TARGETDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(TARGETDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(TARGETDIR))
endif

$(OBJDIR):
	@echo Creating $(OBJDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(OBJDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(OBJDIR))
endif

clean:
	@echo Cleaning luatask
ifeq (posix,$(SHELLTYPE))
	$(SILENT) rm -f  $(TARGET)
	$(SILENT) rm -rf $(OBJDIR)
else
	$(SILENT) if exist $(subst /,\\,$(TARGET)) del $(subst /,\\,$(TARGET))
	$(SILENT) if exist $(subst /,\\,$(OBJDIR)) rmdir /s /q $(subst /,\\,$(OBJDIR))
endif

prebuild:
	$(PREBUILDCMDS)

prelink:
	$(PRELINKCMDS)

ifneq (,$(PCH))
$(GCH): $(PCH)
	@echo $(notdir $<)
	$(SILENT) $(CC) $(CFLAGS) -o $@ -c $<
endif

$(OBJDIR)/ltask.o: ../../lua/luatask/src/ltask.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(CFLAGS) -o $@ -c $<
$(OBJDIR)/queue.o: ../../lua/luatask/src/queue.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(CFLAGS) -o $@ -c $<
$(OBJDIR)/syncos.o: ../../lua/luatask/src/syncos.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(CFLAGS) -o $@ -c $<

-include $(OBJECTS:%.o=%.d)
