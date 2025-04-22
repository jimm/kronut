NAME = kronut
# DEBUG = -DDEBUG -DDEBUG_STDERR
MACOS_VER = 10.9
CPPFLAGS += -std=c++14 -mmacosx-version-min=$(MACOS_VER) -MD -MP -g $(DEBUG)
LIBS = -lrtmidi
TESTLIBS = -lCatch2 -lCatch2Main
LDFLAGS += $(LIBS)

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin

SRC = $(wildcard src/*.cpp)
OBJS = $(SRC:%.cpp=%.o)
TEST_SRC = $(wildcard test/*.cpp)
TEST_OBJS = $(TEST_SRC:%.cpp=%.o)
TEST_OBJ_FILTERS = src/$(NAME).o

CATCH_CATEGORY ?= ""

.PHONY: all test install uninstall tags clean distclean

all: $(NAME)

%.d: %.cpp
	@set -e; rm -f $@; \
	 $(CXX) -MM $(CPPFLAGS) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$

include $(SRC:.cpp=.d)

$(NAME): $(OBJS)
	echo $(CXX)
	$(CXX) $(LDFLAGS) -o $@ $^

test: $(NAME)_test
	./$(NAME)_test --colour-mode none $(CATCH_CATEGORY)

$(NAME)_test:	$(OBJS) $(TEST_OBJS)
	$(CXX) $(LDFLAGS) $(TESTLIBS) -o $@ $(filter-out $(TEST_OBJ_FILTERS),$^)

install:	$(NAME)
	install ./$(NAME) $(bindir)

uninstall:
	rm -f $(bindir)/$(NAME)

tags:	TAGS

TAGS:	$(SRC)
	etags $(SRC)

clean:
	rm -f $(NAME) $(NAME)_test src/*.o test/*.o

distclean: clean
	rm -f src/*.d test/*.d
