# ============================================================
#  Makefile  –  Space Shooter (Rage-Fang Edition)
#
#  ── BEFORE BUILDING ─────────────────────────────────────────
#  1. Get stb_image.h and place it in this folder:
#       pacman -S mingw-w64-x86_64-stb
#       cp /c/msys64/mingw64/include/stb/stb_image.h .
#
#  2. Place these PNG files next to the compiled .exe:
#       ship.png          –  player ship (nose pointing RIGHT)
#       normal_alien.png  –  normal enemy sprite
#       boss_alien.png    –  boss enemy sprite
#
#  ── BUILD ────────────────────────────────────────────────────
#    MSYS2 MINGW64:   make
#    Linux / macOS:   make PLATFORM=linux
#
#  ── RUN ──────────────────────────────────────────────────────
#    Copy DLLs (first time only):
#      cp /c/msys64/mingw64/bin/glew32.dll .
#      cp /c/msys64/mingw64/bin/glfw3.dll  .
#    Then:
#      ./space_shooter.exe
# ============================================================

PLATFORM ?= windows

ifeq ($(PLATFORM), linux)
    CXX    = g++
    LIBS   = -lGL -lGLEW -lglfw
    TARGET = space_shooter
else
    CXX    = /c/msys64/mingw64/bin/g++
    LIBS   = -lglew32 -lglfw3 -lopengl32 -lgdi32 -mconsole
    TARGET = space_shooter.exe
endif

# -I. makes the compiler search the current folder for headers
# (needed so #include "stb_image.h" resolves correctly)
CXXFLAGS = -std=c++17 -O2 -Wall -I.

# All translation units
SRCS = main.cpp       \
       game.cpp       \
       renderer.cpp   \
       alien.cpp      \
       player.cpp     \
       projectile.cpp \
       common.cpp

OBJS = $(SRCS:.cpp=.o)

# ── Default target ───────────────────────────────────────────
all: $(TARGET)

# Link all object files into the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo ""
	@echo "  Build successful → $(TARGET)"
	@echo "  Remember: ship.png, normal_alien.png, boss_alien.png"
	@echo "  must be in the same folder as the .exe"
	@echo ""

# Compile each .cpp to a .o object file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Remove build artefacts
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
