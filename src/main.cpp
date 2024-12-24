#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

#include  "MatrixConfig.h"

// Cell structure to represent each character in the vertical line
struct Cell {
    char character;
    int age; // Age of the character (used for fading effect)
    Cell(char c, int a) : character(c), age(a) {}
};

// SDLSetup class: Handles SDL initialization and cleanup
class SDLSetup {
public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;

    SDLSetup() : window(nullptr), renderer(nullptr), font(nullptr) {}

    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            printf("SDL_Init Error: %s\n", SDL_GetError());
            return false;
        }

        if (TTF_Init() != 0) {
            printf("TTF_Init Error: %s\n", TTF_GetError());
            SDL_Quit();
            return false;
        }

        window = SDL_CreateWindow("SDL2 Falling Characters", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                  WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) {
            printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
            TTF_Quit();
            SDL_Quit();
            return false;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
            SDL_DestroyWindow(window);
            TTF_Quit();
            SDL_Quit();
            return false;
        }

        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24);
        if (!font) {
            printf("TTF_OpenFont Error: %s\n", TTF_GetError());
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            TTF_Quit();
            SDL_Quit();
            return false;
        }

        return true;
    }

    void cleanup() {
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
    }
};

// VerticalLineGenerator class: Handles the logic of falling characters
class VerticalLineGenerator {
private:
    int xPosition;
    std::vector<Cell> lineCells;
    int rows;
    float leadPosition;
    float descentSpeed;
    float fadeRate;

public:
    VerticalLineGenerator(int xPos, float speed, float fade)
        : xPosition(xPos), leadPosition(0), descentSpeed(speed), fadeRate(fade) {
        rows = WINDOW_HEIGHT / (CELL_SIZE / 2);
        srand((unsigned int)time(NULL));
        for (int i = 0; i < rows; ++i) {
            char randomChar = (rand() % 95) + 32;  // Random ASCII character (32 to 126)
            lineCells.emplace_back(randomChar, -1); // Age -1 means invisible
        }
        lineCells[0].age = 0;  // Initialize the lead character
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
            leadPosition += descentSpeed;  // Smooth movement
            if (leadPosition >= rows) leadPosition = rows - 1;

            // Reset the character when it moves down
            lineCells[static_cast<int>(leadPosition)].character = (rand() % 95) + 32;  // New random character
            lineCells[static_cast<int>(leadPosition)].age = 0;                           // Reset age
        }
    }

    void render(SDL_Renderer* renderer, TTF_Font* font) {
        for (int y = 0; y < rows; ++y) {
            if (lineCells[y].age == -1) continue;  // Skip invisible cells

            int alpha = 255 - static_cast<int>(lineCells[y].age * fadeRate); // Fade effect
            if (alpha < 0) {
                lineCells[y].age = -1; // Mark as invisible
                continue;
            }

            SDL_Color color = {255, 255, 255, alpha};
            char text[2] = {lineCells[y].character, '\0'};  // Convert char to string

            SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, color);
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_SetTextureAlphaMod(textTexture, alpha);

            int textW, textH;
            SDL_QueryTexture(textTexture, NULL, NULL, &textW, &textH);

            SDL_Rect dstRect = {xPosition * CELL_SIZE + (CELL_SIZE - textW) / 2,
                                static_cast<int>(y * (CELL_SIZE / 2) + (CELL_SIZE - textH) / 2),
                                textW, textH};

            SDL_RenderCopy(renderer, textTexture, NULL, &dstRect);

            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);
        }
    }

    void setDescentSpeed(float speed) { descentSpeed = speed; }
    void setFadeRate(float rate) { fadeRate = rate; }
};

// Game class: Handles game logic and event handling
class Game {
private:
    SDLSetup sdlSetup;
    VerticalLineGenerator verticalLine;

public:
    Game() : verticalLine(5, 0.2f, 2.95f) {}

    bool init() {
        return sdlSetup.init();
    }

    void handleEvents(SDL_Event& event, bool& running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_UP) {
                    verticalLine.setDescentSpeed(0.5f);
                } else if (event.key.keysym.sym == SDLK_DOWN) {
                    verticalLine.setDescentSpeed(0.1f);
                } else if (event.key.keysym.sym == SDLK_RIGHT) {
                    verticalLine.setFadeRate(0.1f);
                } else if (event.key.keysym.sym == SDLK_LEFT) {
                    verticalLine.setFadeRate(0.5f);
                }
            }
        }
    }

    void run() {
        bool running = true;
        SDL_Event event;
        Uint32 frameStart;
        int frameTime;

        while (running) {
            frameStart = SDL_GetTicks();
            handleEvents(event, running);

            SDL_SetRenderDrawColor(sdlSetup.renderer, 0, 0, 0, 255);  // Black background
            SDL_RenderClear(sdlSetup.renderer);

            verticalLine.update();
            verticalLine.render(sdlSetup.renderer, sdlSetup.font);

            SDL_RenderPresent(sdlSetup.renderer);

            frameTime = SDL_GetTicks() - frameStart;
            if (frameTime < FRAME_DELAY) {
                SDL_Delay(FRAME_DELAY - frameTime);
            }
        }
    }

    void cleanup() {
        sdlSetup.cleanup();
    }
};

// Main function
int main() {
    Game game;

    if (!game.init()) {
        return 1;
    }

    game.run();
    game.cleanup();

    return 0;
}
