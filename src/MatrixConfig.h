#ifndef MATRIXCONFIG_H
#define MATRIXCONFIG_H

// Window dimensions
constexpr int WINDOW_WIDTH = 1800;
constexpr int WINDOW_HEIGHT = 1600;

// Cell dimensions
constexpr int CELL_SIZE = 40;

// Frame timing
constexpr int TARGET_FPS = 60;
constexpr int FRAME_DELAY = 1000 / TARGET_FPS;

// Vertical line settings
constexpr float DEFAULT_DESCENT_SPEED = 0.2f;
constexpr float DEFAULT_FADE_RATE = 2.95f;

#endif // MATRIXCONFIG_H
