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
  OBJDIR     = obj/Release/wxbindhtml
  TARGETDIR  = ../../lib/Release
  TARGET     = $(TARGETDIR)/libwxbindhtml.a
  DEFINES   += -DNDEBUG -D_LIB -DUNICODE -D_UNICODE
  INCLUDES  += -I../../lua/wxbind/setup -I../../lua/lua/include -I../../lua
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -O2 `wx-config --release --unicode --static --cxxflags`
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -s -L../../bin/Release -L../../lib/Release
  LIBS      += -lwxbindcore
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += ../../lib/Release/libwxbindcore.a
  LINKCMD    = $(AR) -rcs $(TARGET) $(OBJECTS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),debug)
  OBJDIR     = obj/Debug/wxbindhtml
  TARGETDIR  = ../../lib/Debug
  TARGET     = $(TARGETDIR)/libwxbindhtml.a
  DEFINES   += -DDEBUG -D_DEBUG -D_LIB -DUNICODE -D_UNICODE -D__WXDEBUG__
  INCLUDES  += -I../../lua/wxbind/setup -I../../lua/lua/include -I../../lua
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g `wx-config --debug --unicode --static --cxxflags`
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -L../../bin/Debug -L../../lib/Debug
  LIBS      += -lwxbindcore
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += ../../lib/Debug/libwxbindcore.a
  LINKCMD    = $(AR) -rcs $(TARGET) $(OBJECTS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

OBJECTS := \
	$(OBJDIR)/wxhtml_bind.o \
	$(OBJDIR)/wxhtml_wxlhtml.o \
	$(OBJDIR)/dummy.o \

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
	@echo Linking wxbindhtml
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
	@echo Cleaning wxbindhtml
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
	$(SILENT) $(CXX) $(CXXFLAGS) -o $@ -c $<
endif

$(OBJDIR)/wxhtml_bind.o: ../../lua/wxbind/src/wxhtml_bind.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o $@ -c $<
$(OBJDIR)/wxhtml_wxlhtml.o: ../../lua/wxbind/src/wxhtml_wxlhtml.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o $@ -c $<
$(OBJDIR)/dummy.o: ../../lua/wxbind/src/dummy.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o $@ -c $<

-include $(OBJECTS:%.o=%.d)
