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

# I tried using the .cpp -> .d dependency rule and include that the Gnu make
# docs recommend for automatic dependency generation, but sometimes compiles
# failed due to "missing delimiter" in the .d files.

file_editor.cpp: file_editor.h kronos.h
file_editor.h: kronos.h
kronos.cpp: consts.h kronos.h midi_data.h kstring.h set_list.h utils.h
kronos.h: consts.h set_list.h midi_data.h kstring.h
kronut.cpp: slot.h kronos.h file_editor.h test_editor.h
kstring.cpp: kstring.h utils.h
kstring.h: midi_data.h
midi_data.cpp: midi_data.h utils.h
# midi_data.h:
set_list_file.cpp: set_list_file.h
set_list_file.h: kstring.h
set_list_wrapper.cpp: set_list_wrapper.h
set_list_wrapper.h: set_list.h struct_wrapper.h
set_list.h: slot.h
slot_wrapper.cpp: slot_wrapper.h
slot_wrapper.h: slot.h struct_wrapper.h
# slot.h:
struct_wrapper.cpp: struct_wrapper.h
struct_wrapper.h:
text_editor.cpp: text_editor.h
text_editor.h: kronos.h kstring.h
utils.cpp: utils.h
# utils.h:

$(NAME): $(OBJS)
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
