
UNAME_S := $(shell uname -s)

ifeq ($(OS),Windows_NT)
	CC 			:= cl
	CFLAGS 	:= /nologo /W4 /LD /D ADBC_EXPORTING /wd4100
	SRCS 		:= src\tiny.c
	TARGET 	:= /Fe:tiny.dll
	RM 			:= del /Q
else
		CC 			:= cc
		CFLAGS 	:= -Wall -Wextra -O2 -fPIC -Wno-unused
		SRCS 		:= src/tiny.c
		RM 			:= rm

    ifeq ($(UNAME_S),Linux)
			LDFLAGS := -shared
			TARGET 	:= -o libtiny.so
    else ifeq ($(UNAME_S),Darwin)
			LDFLAGS := -dynamiclib
			TARGET 	:= -o libtiny.dylib
    endif
	CFLAGS 	:= -Wall -Wextra -O2 -fPIC -Wno-unused-parameter
endif


.PHONY: all build docs clean

all: build docs

build: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCS) $(TARGET)

docs:
	npx docco src/tiny.c && mv docs/src/tiny.html docs/index.html && sed -i.bak 's/..\/docco\.css/docco.css/g' docs/index.html && rm docs/index.html.bak

clean:
	$(RM) $(TARGET)
