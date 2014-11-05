.PHONY : all clean

CXXFLAGS = $(EAGLE_COMPILE_OPT) -g -Wall -Wno-deprecated -fPIC
SHARED = -shared -fPIC

INC_PATH := \
	-I. \
	-I../base \
	-I../third_party_include \
	-I../third_party_include/lua \
	-I../third_party_include/log4cxx

LIB_LUAS := libluas.so
all: $(LIB_LUAS)

$(LIB_LUAS): luas.cpp luas_reader.cpp luas_reader.h luas.h lua_inc.h
	@rm -rf $(LIB_LUAS)
	$(CXX) $(SHARED) $(CXXFLAGS) -o $@ $^ $(INC_PATH)
clean:
	@rm -rf $(OBJS) $(LIB_LUAS) .*.d
