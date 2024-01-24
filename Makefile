CXXFLAGS := -std=c++11 -Wall -Wextra -pedantic-errors -O2 $(CXXFLAGS)
LDFLAGS := -Wl,-s $(LDFLAGS)
ifdef MINGW_PREFIX
  LDFLAGS := -municode -static $(LDFLAGS)
  TARGET ?= psisimux.exe
else
  LDFLAGS := $(LDFLAGS)
  TARGET ?= psisimux
endif

all: $(TARGET)
$(TARGET): psisimux.cpp b24captionutil.cpp b24captionutil.hpp psiarchivereader.cpp psiarchivereader.hpp readonlympeg4file.cpp readonlympeg4file.hpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH) -o $@ psisimux.cpp b24captionutil.cpp psiarchivereader.cpp readonlympeg4file.cpp
clean:
	$(RM) $(TARGET)
