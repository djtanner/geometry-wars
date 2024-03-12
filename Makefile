CXX		:=	g++
OUTPUT	:=	sfmlgame

SFML_DIR	:=	/opt/homebrew/Cellar/sfml/2.6.1

CXX_FLAGS	:=	-std=c++17 -O3
INCLUDES	:=	-I./ -I$(SFML_DIR)/include
LDFLAGS		:=	-O3 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -L$(SFML_DIR)/lib

SRC_FILES	:=	$(wildcard *.cpp)
OBJ_FILES	:=	$(SRC_FILES:.cpp=.o)

all: $(OUTPUT)

$(OUTPUT): $(OBJ_FILES) Makefile
	$(CXX) $(OBJ_FILES) $(LDFLAGS) -o $@

.cpp.o:
	$(CXX) -c $(CXX_FLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ_SFMLGAME) ./bin/sfmlgame

run: $(OUTPUT)
	.sfmlgame && cd ..