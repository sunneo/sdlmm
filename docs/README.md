# SDLMM - Simplified SDL Multimedia Library

A lightweight C wrapper library for SDL 1.2 that simplifies multimedia application development with an easy-to-use API for 2D graphics, audio, text rendering, and 3D graphics.

## 🎯 Overview

SDLMM provides a simplified interface to SDL (Simple DirectMedia Layer) with additional features including:
- **2D Graphics**: Drawing primitives, image loading, and pixel manipulation
- **3D Graphics**: Babylon3D-inspired 3D rendering engine with software rasterization
- **Audio**: Wave file playback and audio streaming
- **Text Rendering**: TrueType font support with UTF-8 and wide character support
- **Input Handling**: Mouse, keyboard, and touch event handling
- **Effects**: Mode7 pseudo-3D rendering for retro-style games
- **Async Operations**: Thread-based asynchronous function execution

## ✨ Features

### 2D Graphics
- Basic primitives: pixels, lines, rectangles, circles
- Filled shapes with customizable colors
- Image loading (BMP, PNG, JPG via SDL_image)
- Pixel buffer manipulation
- Transparency and alpha blending support
- Image stretching and scaling

### 3D Graphics (Babylon3D)
- Software-based 3D rasterizer
- Mesh creation and manipulation
- Camera system with position and target
- Depth buffering for proper z-ordering
- Vertex lighting with normal vectors
- Texture mapping support
- Matrix transformations

### Audio
- Direct wave playback (44.1kHz, stereo)
- WAV file loading
- Real-time audio streaming

### Text Rendering
- TrueType font support via SDL_ttf
- ASCII and UTF-8/Unicode text rendering
- Customizable font size and color
- Anti-aliased text

### Input & Events
- Mouse click and motion events
- Touch event support
- Keyboard input with modifiers
- Custom event handlers

### Special Effects
- Mode7 pseudo-3D ground rendering
- Configurable perspective effects
- Useful for racing games and retro effects

## 📋 Prerequisites

### Ubuntu/Debian
```bash
sudo apt-get install libsdl1.2-dev libsdl-image1.2-dev libfreetype6-dev libsdl-ttf2.0-dev
```

### Fedora/RHEL
```bash
sudo dnf install SDL-devel SDL_image-devel freetype-devel SDL_ttf-devel
```

### macOS (Homebrew)
```bash
brew install sdl sdl_image sdl_ttf freetype
```

### Build Tools
- GCC or compatible C compiler
- Make
- OpenMP support (optional, for parallel operations)

## 🚀 Quick Start

### Basic 2D Example

```c
#include "sdlmm.h"

int main() {
    // Initialize screen
    screen(800, 600);
    screentitle("My SDLMM App");
    
    // Main loop
    while(1) {
        // Clear screen
        fillrect(0, 0, 800, 600, 0xFFFFFF);
        
        // Draw a red circle
        fillcircle(400, 300, 50, 0xFF0000);
        
        // Draw text
        drawtext("Hello, SDLMM!", 350, 250, 0x000000);
        
        // Update screen
        flushscreen();
        delay(16); // ~60 FPS
    }
    
    return 0;
}
```

### Compile and Run

```bash
gcc -O2 -I/usr/include/SDL -I/usr/include/freetype2 \
    your_program.c sdlmm.c \
    -lSDL -lm -lpthread -lSDL_ttf -lSDL_image -lfreetype -fopenmp \
    -o your_program

./your_program
```

## 📚 API Reference

### Screen Initialization

```c
void screen(int width, int height);           // Create window
void screentitle(const char* title);          // Set window title
void flushscreen();                           // Update display (vsync)
void setalwaysflush(int doflush);            // Auto-flush after draw
void setusealpha(int usealpha);              // Enable alpha blending
```

### Drawing Functions

```c
void drawpixel(int x, int y, int color);
void drawline(int x1, int y1, int x2, int y2, int color);
void drawrect(int x, int y, int w, int h, int color);
void drawcircle(int x, int y, double r, int color);

void fillrect(int x, int y, int w, int h, int color);
void fillcircle(int x, int y, double r, int color);
void fillxy(int x, int y, int color);  // Flood fill
```

### Image Operations

```c
void loadimage(const char* filename, int** ret, int* w, int* h);
void drawpixels(int* pixels, int x, int y, int w, int h);
void drawpixels2(int* pixels, int x, int y, int w, int h, int transkey);
void stretchpixels(const int* pixels, int w, int h, int* output, int w2, int h2);
void copyscreen(int** ret, int x, int y, int w, int h);
```

### Text Rendering

```c
void settextfont(const char* font, int fontsize);
void drawtext(const char* str, int x, int y, int color);
void drawtextw(const wchar_t* str, int x, int y, int color);  // Unicode
```

### Audio

```c
void playwave(short* wave, int len);  // Play PCM audio (44.1kHz stereo)
void loadwav(const char* filename, short** wav, unsigned int* len);
```

### Event Handling

```c
void setontouch(void (*fnc)(int x, int y, int on));
void setonmotion(void (*fnc)(int x, int y, int on));
void setonmouse(void(*fnc)(int x, int y, int on, int btn));
void setonkey(void(*fnc)(int key, int ctrl, int on));
```

### Threading

```c
int run_async(void (*fnc)(void*), void* param);
void post_async(void (*fnc)(void*), void* param);
void delay(int mills);
```

### Mode7 Rendering

```c
void mode7render(float angle, int vx, int vy, int* bg, int bw, int bh, 
                 int tx, int ty, int w, int h);
void* mode7render_create_conf(float groundFactor, float xFactor, 
                               float yFactor, int scanlineJump);
void mode7render2(void* mode, float angle, int vx, int vy, int* bg, 
                  int bw, int bh, int tx, int ty, int w, int h);
```

### Babylon3D API

See `babylon3D.h` for the complete 3D rendering API including:
- `Device*` - Rendering device with back/depth buffers
- `Mesh*` - 3D meshes with vertices and faces
- `Camera` - Camera positioning and targeting
- `Texture*` - Texture loading and mapping
- Matrix transformations and vector operations

## 📖 Examples

The `exams/` directory contains various examples demonstrating library features:

### 2D Graphics Examples
- **DrawCircleOnClick.c** - Interactive circle drawing on mouse click/drag
- **DrawRectOnClick.c** - Rectangle drawing with mouse input
- **AlphaDraw.c** - Alpha blending demonstration
- **farn.c** - Barnsley fern fractal generator
- **particles.c** / **particles2.c** - Particle system effects

### Animation & Effects
- **animation.c** - Sprite animation system
- **mode7.c** - Mode7 pseudo-3D ground rendering
- **bubbleSort.c** - Visual bubble sort animation

### Physics & Simulation
- **nbody.c** - N-body gravitational simulation
- **test.c** - N-body physics with OpenMP acceleration
- **sor.c** - Successive over-relaxation simulation

### 3D Graphics
- **babylon3D_cube.c** - Rotating 3D cube with lighting

### Audio
- **PlayWave.c** - Synthesized wave generation and playback
- **PlayWaveFile.c** - WAV file playback

### Text & UI
- **PressKeyDrawText.c** - Keyboard input and text rendering
- **windowdemo.c** - Window management demonstration

### Games
- **missilecmd.c** - Missile Command style game
- **CombineEveryThing.c** - Comprehensive feature showcase

## 🔨 Building

### Build the Library

```bash
make
```

### Build Examples

```bash
# Build specific example
gcc -O2 -I/usr/include/SDL -I/usr/include/freetype2 -I./ \
    exams/DrawCircleOnClick.c sdlmm.c \
    -lSDL -lm -lpthread -lSDL_ttf -lSDL_image -lfreetype -fopenmp \
    -o DrawCircleOnClick

# Build 3D example
make babylon3D_cube
```

### Compiler Flags

- `-O2` - Optimization level 2 (recommended)
- `-fopenmp` - Enable OpenMP for parallel operations
- `-I/usr/include/SDL` - SDL headers
- `-I/usr/include/freetype2` - FreeType headers
- `-lSDL -lSDL_ttf -lSDL_image -lfreetype -lm -lpthread` - Required libraries

## 🧪 Testing

Run the test program (N-body simulation):

```bash
make test
./test
```

## 🏗️ Project Structure

```
sdlmm/
├── sdlmm.c                 # Main library implementation
├── sdlmm.h                 # Public API header
├── libsdlmmWrap.c          # Additional wrapper functions
├── libsdlmmWrap.h          # Wrapper header
├── babylon3D.c             # 3D rendering engine
├── babylon3D.h             # 3D API header
├── anime_engine.h          # Animation engine header
├── default_anime_engine.c  # Animation implementation
├── test.c                  # Test/demo program
├── Makefile                # Build configuration
├── exams/                  # Example programs
│   ├── *.c                 # Various examples
│   └── BABYLON3D_README.md # 3D examples documentation
└── devcpp-sdlmm/          # Dev-C++ project files
```

## 🔒 Memory Management

SDLMM has been optimized to prevent memory leaks:
- All SDL surfaces are properly freed after use
- FreeType font caches are cleaned up on exit
- Thread parameters are properly deallocated
- See `OPTIMIZATION_SUMMARY.md` for detailed memory leak fixes

## 🐛 Known Limitations

- Built for SDL 1.2 (deprecated but stable; SDL 2.0+ migration recommended for new projects)
- 3D rendering is software-based (no GPU acceleration)
- Audio format limited to 44.1kHz stereo PCM
- No built-in sprite or scene management

## 📝 License

Copyright 2024, Sunneo IceCold

Licensed under the BSD 3-Clause License. See [LICENSE](LICENSE) file for details.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the conditions in the LICENSE file are met.

## 🤝 Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes with clear commit messages
4. Test your changes thoroughly
5. Submit a pull request

### Contribution Ideas
- Port to SDL 2.0
- Add more 3D primitives (sphere, cylinder, etc.)
- Implement GPU-accelerated rendering
- Add more audio formats
- Create additional examples
- Improve documentation

## 🔗 Resources

- [SDL 1.2 Documentation](https://www.libsdl.org/release/SDL-1.2.15/docs/)
- [SDL_ttf Documentation](https://www.libsdl.org/projects/SDL_ttf/docs/)
- [SDL_image Documentation](https://www.libsdl.org/projects/SDL_image/docs/)
- [FreeType Documentation](https://freetype.org/freetype2/docs/)

## 📧 Contact

For questions, issues, or suggestions, please open an issue on the GitHub repository.

## 🙏 Acknowledgments

- SDL team for the excellent multimedia library
- FreeType project for font rendering
- Babylon.js for 3D engine inspiration

---

**Happy coding with SDLMM!** 🎮✨
