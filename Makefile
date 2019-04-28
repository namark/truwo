
override CPPFLAGS	+= --std=c++1z
override CPPFLAGS	+= -MMD -MP
override CPPFLAGS	+= $(shell cat .cxxflags | xargs)
override CPPFLAGS	+= -I./include
override LDFLAGS	+= $(shell cat .ldflags | xargs)
override LDFLAGS	+= -L./lib
override LDLIBS		+= -lSDL2main -lSDL2 -lpthread -lGL -lGLEW

NAME := melno

SRCDIR	:= source
TEMPDIR	:= temp
DISTDIR	:= out
SOURCES	:= $(wildcard $(SRCDIR)/*.cpp)
TARGET	:= $(DISTDIR)/$(NAME)
OBJECTS	:= $(SOURCES:%.cpp=$(TEMPDIR)/%.o)
LOCALIB	:= $(wildcard lib/*.a)
DEPENDS	:= $(OBJECTS:.o=.d)

build: $(TARGET)

$(TARGET): $(OBJECTS) $(LOCALIB) | $(DISTDIR)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LOCALIB) $(LDLIBS) -o $@

$(TEMPDIR)/%.o: %.cpp | $(TEMPDIR)
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

$(TEMPDIR):
	@mkdir $@

$(DISTDIR):
	@mkdir $@

clean:
	@rm $(DEPENDS) 2> /dev/null || true
	@rm $(OBJECTS) 2> /dev/null || true
	@rmdir -p $(TEMPDIR) 2> /dev/null || true
	@echo Temporaries cleaned!

distclean: clean
	@rm $(TARGET) 2> /dev/null || true
	@rmdir -p $(DISTDIR) 2> /dev/null || true
	@echo All clean!

-include $(DEPENDS)

.PRECIOUS : $(OBJECTS)
.PHONY : clean distclean
