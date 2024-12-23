#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

#define WINDOW_WIDTH 1800
#define WINDOW_HEIGHT 1600
#define CELL_SIZE 40  // Size of each grid cell

const int TARGET_FPS = 60;             // Target frames per second
const int FRAME_DELAY = 1000 / TARGET_FPS; // Delay per frame in milliseconds

struct Cell {
    char character;
    int age; // Age of the character (used for fading effect)

    Cell(char c, int a) : character(c), age(a) {}
};

class VerticalLineGenerator {
private:
    int xPosition;                    // X position in terms of grid cells
    std::vector<Cell> lineCells;      // Stores cells for the vertical line
    int rows;                         // Total number of rows in the grid
    float leadPosition;               // Current position of the lead character (float for smooth movement)
    float descentSpeed;               // Controls the speed of the lead character's descent (0-1 range)
    float fadeRate;                   // Controls how quickly the age-to-alpha gradient fades

public:
    VerticalLineGenerator(int xPos, float speed, float fade) 
        : xPosition(xPos), leadPosition(0), descentSpeed(speed), fadeRate(fade) {
        rows = WINDOW_HEIGHT / CELL_SIZE;
        srand((unsigned int)time(NULL));
        // Initialize the cells with random characters
        for (int i = 0; i < rows; ++i) {
            char randomChar = (rand() % 95) + 32; // Random ASCII character (32 to 126)
            lineCells.emplace_back(randomChar, -1); // Age -1 means invisible
        }
        lineCells[0].age = 0; // Initialize the lead character
    }

    void update() {
        // Increment the age of all characters
        for (auto& cell : lineCells) {
            if (cell.age >= 0) {
                cell.age++;
            }
        }

        // Move the lead character down based on descent speed
        if (leadPosition < rows - 1) {
            leadPosition += descentSpeed;  // Now descentSpeed is a fraction and works smoothly
            if (leadPosition >= rows) leadPosition = rows - 1;  // Prevent it from going out of bounds

            // Reset the character when it moves down
            lineCells[static_cast<int>(leadPosition)].character = (rand() % 95) + 32;  // New random character
            lineCells[static_cast<int>(leadPosition)].age = 0;                           // Reset age
        }
    }

    void render(SDL_Renderer* renderer, TTF_Font* font) {
        for (int y = 0; y < rows; ++y) {
            if (lineCells[y].age == -1) {
                continue; // Skip invisible cells
            }

            // Calculate alpha based on age, considering the fadeRate
            int alpha = 255 - static_cast<int>(lineCells[y].age * fadeRate); // Fade out slower or faster based on fadeRate
            if (alpha < 0) {
                lineCells[y].age = -1; // Mark as invisible
                continue;
            }

            SDL_Color color = {255, 255, 255, alpha};

            char text[2] = {lineCells[y].character, '\0'}; // Convert char to string

            // Render the text to a surface
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, color);
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_SetTextureAlphaMod(textTexture, alpha);

            // Get text dimensions
            int textW, textH;
            SDL_QueryTexture(textTexture, NULL, NULL, &textW, &textH);

            // Determine where to render the character
            SDL_Rect dstRect = {xPosition * CELL_SIZE + (CELL_SIZE - textW) / 2,
                                y * CELL_SIZE + (CELL_SIZE - textH) / 2,
                                textW, textH};

            // Render the texture
            SDL_RenderCopy(renderer, textTexture, NULL, &dstRect);

            // Clean up the surface and texture
            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);
        }
    }

    void setDescentSpeed(float speed) {
        descentSpeed = speed;
    }

    void setFadeRate(float rate) {
        fadeRate = rate;  // Set the fade rate to control fading speed
    }
};

int main(int /*argc*/, char* /*argv*/[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() != 0) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // Create SDL window
    SDL_Window* window = SDL_CreateWindow("SDL2 Falling Characters",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          SDL_WINDOW_SHOWN);

    if (!window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Create SDL renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24);
    if (!font) {
        printf("TTF_OpenFont Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Create VerticalLineGenerator object with descent speed 0.1 (slow speed) and fade rate 0.25
    VerticalLineGenerator verticalLine(5, 0.2, 2.95); // Set to column 5, slow descent speed, and fade rate of 0.25

    // Main loop
    int running = 1;
    SDL_Event event;

    Uint32 frameStart;
    int frameTime;

    while (running) {
        // Start of frame
        frameStart = SDL_GetTicks();

        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            // Adjust descent speed and fade rate based on key press (for demonstration)
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_UP) {
                    verticalLine.setDescentSpeed(0.5f); // Increase speed
                } else if (event.key.keysym.sym == SDLK_DOWN) {
                    verticalLine.setDescentSpeed(0.1f); // Slow down
                } else if (event.key.keysym.sym == SDLK_RIGHT) {
                    verticalLine.setFadeRate(0.1f); // Make fading faster
                } else if (event.key.keysym.sym == SDLK_LEFT) {
                    verticalLine.setFadeRate(0.5f); // Make fading slower
                }
            }
        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
        SDL_RenderClear(renderer);

        // Update and render the vertical line
        verticalLine.update();
        verticalLine.render(renderer, font);

        // Present the renderer
        SDL_RenderPresent(renderer);

        // Calculate frame time
        frameTime = SDL_GetTicks() - frameStart;

        // If frame finished early, delay to maintain framerate
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    // Clean up
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
