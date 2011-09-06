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
  OBJDIR     = obj/Release/yajl
  TARGETDIR  = ../../bin/Release
  TARGET     = $(TARGETDIR)/libyajl.so
  DEFINES   += -DNDEBUG -DYAJL_BUILD
  INCLUDES  += -I../../yajl/build/yajl-2.0.2/include
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -O2 -fPIC -std=c99 -pedantic -Wpointer-arith -Wno-format-y2k -Wstrict-prototypes -Wmissing-declarations -Wnested-externs -Wextra -Wundef -Wwrite-strings -Wold-style-definition -Wredundant-decls -Wno-unused-parameter -Wno-sign-compare -Wmissing-prototypes -O2 -Wuninitialized
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -s -shared -L../../bin/Release -L../../lib/Release
  LIBS      += -ldl
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += 
  LINKCMD    = $(CC) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),debug)
  OBJDIR     = obj/Debug/yajl
  TARGETDIR  = ../../bin/Debug
  TARGET     = $(TARGETDIR)/libyajl.so
  DEFINES   += -DDEBUG -D_DEBUG -DYAJL_BUILD
  INCLUDES  += -I../../yajl/build/yajl-2.0.2/include
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g -fPIC -std=c99 -pedantic -Wpointer-arith -Wno-format-y2k -Wstrict-prototypes -Wmissing-declarations -Wnested-externs -Wextra -Wundef -Wwrite-strings -Wold-style-definition -Wredundant-decls -Wno-unused-parameter -Wno-sign-compare -Wmissing-prototypes -O2 -Wuninitialized
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -shared -L../../bin/Debug -L../../lib/Debug
  LIBS      += -ldl
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += 
  LINKCMD    = $(CC) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

OBJECTS := \
	$(OBJDIR)/yajl.o \
	$(OBJDIR)/yajl_alloc.o \
	$(OBJDIR)/yajl_buf.o \
	$(OBJDIR)/yajl_encode.o \
	$(OBJDIR)/yajl_gen.o \
	$(OBJDIR)/yajl_lex.o \
	$(OBJDIR)/yajl_parser.o \
	$(OBJDIR)/yajl_tree.o \
	$(OBJDIR)/yajl_version.o \

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
	@echo Linking yajl
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
	@echo Cleaning yajl
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

$(OBJDIR)/yajl.o: ../../yajl/src/yajl.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(CFLAGS) -o $@ -c $<
$(OBJDIR)/yajl_alloc.o: ../../yajl/src/yajl_alloc.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(CFLAGS) -o $@ -c $<
$(OBJDIR)/yajl_buf.o: ../../yajl/src/yajl_buf.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(CFLAGS) -o $@ -c $<
$(OBJDIR)/yajl_encode.o: ../../yajl/src/yajl_encode.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(CFLAGS) -o $@ -c $<
$(OBJDIR)/yajl_gen.o: ../../yajl/src/yajl_gen.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(CFLAGS) -o $@ -c $<
$(OBJDIR)/yajl_lex.o: ../../yajl/src/yajl_lex.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(CFLAGS) -o $@ -c $<
$(OBJDIR)/yajl_parser.o: ../../yajl/src/yajl_parser.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(CFLAGS) -o $@ -c $<
$(OBJDIR)/yajl_tree.o: ../../yajl/src/yajl_tree.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(CFLAGS) -o $@ -c $<
$(OBJDIR)/yajl_version.o: ../../yajl/src/yajl_version.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(CFLAGS) -o $@ -c $<

-include $(OBJECTS:%.o=%.d)
