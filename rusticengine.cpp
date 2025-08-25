#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <cmath>

struct Point3D {
    float x, y, z;
};

struct Triangle {
    Point3D p1, p2, p3;
};

// Rotation and projection with camera
SDL_Point project(const Point3D& p, int cx, int cy, float fov, const Point3D& camera, float yaw, float pitch) {
    // Translate point relative to camera
    float x = p.x - camera.x;
    float y = p.y - camera.y;
    float z = p.z - camera.z;

    // Yaw (Y axis rotation)
    float cosYaw = std::cos(yaw);
    float sinYaw = std::sin(yaw);
    float xz = x * cosYaw - z * sinYaw;
    float zz = x * sinYaw + z * cosYaw;

    // Pitch (X axis rotation)
    float cosPitch = std::cos(pitch);
    float sinPitch = std::sin(pitch);
    float yz = y * cosPitch - zz * sinPitch;
    float zz2 = y * sinPitch + zz * cosPitch;

    float factor = fov / (zz2 + fov);
    return {
        static_cast<int>(cx + xz * factor),
        static_cast<int>(cy + yz * factor)
    };
}

int main() {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cerr << "Couldn't initialize SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "3D Cube Perspective Projection",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, 0
    );
    if (!window) {
        std::cerr << "Couldn't create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Couldn't create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int cx = 1280 / 2;
    int cy = 720 / 2;
    float fov = 500.0f;

    // Camera/player position and rotation
    Point3D camera = {0, 0, 0};
    float yaw = 0.0f;   // Y axis (left/right)
    float pitch = 0.0f; // X axis (up/down)

    // Cube setup
    std::vector<Point3D> cube1 = {
        {-100, -100, -100}, // 0 - Bottom face
        {100, -100, -100},  // 1 - Bottom face
        {100, 100, -100},   // 2 - Bottom face
        {-100, 100, -100},  // 3 - Bottom face
        {-100, -100, 100},  // 4 - Top face
        {100, -100, 100},   // 5 - Top face
        {100, 100, 100},    // 6 - Top face
        {-100, 100, 100}    // 7 - Top face
    };

    bool quit = false;
    SDL_Event e;
    const float moveSpeed = 16.0f;
    const float rotSpeed = 0.05f;

    // Key state variables
    bool key_w = false, key_a = false, key_s = false, key_d = false;
    bool key_q = false, key_e = false;
    bool key_left = false, key_right = false, key_up = false, key_down = false;

    while (!quit) {
        // Handle SDL events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                std::cout << "[Event] Quit event received.\n";
                quit = true;
            }
            if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
                bool pressed = (e.type == SDL_KEYDOWN);
                switch (e.key.keysym.sym) {
                    case SDLK_w: key_w = pressed; std::cout << "[Event] W " << (pressed ? "pressed" : "released") << "\n"; break;
                    case SDLK_a: key_a = pressed; std::cout << "[Event] A " << (pressed ? "pressed" : "released") << "\n"; break;
                    case SDLK_s: key_s = pressed; std::cout << "[Event] S " << (pressed ? "pressed" : "released") << "\n"; break;
                    case SDLK_d: key_d = pressed; std::cout << "[Event] D " << (pressed ? "pressed" : "released") << "\n"; break;
                    case SDLK_q: key_q = pressed; std::cout << "[Event] Q " << (pressed ? "pressed" : "released") << "\n"; break;
                    case SDLK_e: key_e = pressed; std::cout << "[Event] E " << (pressed ? "pressed" : "released") << "\n"; break;
                    case SDLK_LEFT:  key_left = pressed; std::cout << "[Event] LEFT " << (pressed ? "pressed" : "released") << "\n"; break;
                    case SDLK_RIGHT: key_right = pressed; std::cout << "[Event] RIGHT " << (pressed ? "pressed" : "released") << "\n"; break;
                    case SDLK_UP:    key_up = pressed; std::cout << "[Event] UP " << (pressed ? "pressed" : "released") << "\n"; break;
                    case SDLK_DOWN:  key_down = pressed; std::cout << "[Event] DOWN " << (pressed ? "pressed" : "released") << "\n"; break;
                }
            }
        }

        // Debug: print camera and angles if any movement/rotation key is pressed
        if (key_w || key_a || key_s || key_d || key_q || key_e ||
            key_left || key_right || key_up || key_down) {
            std::cout << "[Debug] Camera: (" << camera.x << ", " << camera.y << ", " << camera.z
                      << ")  Yaw: " << yaw << "  Pitch: " << pitch << "\n";
        }

        // Camera rotation
        if (key_left)  yaw   -= rotSpeed;
        if (key_right) yaw   += rotSpeed;
        if (key_up)    pitch -= rotSpeed;
        if (key_down)  pitch += rotSpeed;

        // Camera movement (WASD + QE)
        if (key_a) {
            camera.x -= moveSpeed * std::cos(yaw);
            camera.z += moveSpeed * std::sin(yaw);
        }
        if (key_d) {
            camera.x += moveSpeed * std::cos(yaw);
            camera.z -= moveSpeed * std::sin(yaw);
        }
        if (key_w) {
            camera.x += moveSpeed * std::sin(yaw);
            camera.z += moveSpeed * std::cos(yaw);
        }
        if (key_s) {
            camera.x -= moveSpeed * std::sin(yaw);
            camera.z -= moveSpeed * std::cos(yaw);
        }
        if (key_q) camera.y -= moveSpeed;
        if (key_e) camera.y += moveSpeed;

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Project and draw first cube (blue)
        SDL_Point proj[8];
        for (int i = 0; i < 8; ++i)
            proj[i] = project(cube1[i], cx, cy, fov, camera, yaw, pitch);

        SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255); // Blue
        
        // Draw all cube edges
        SDL_RenderDrawLine(renderer, proj[0].x, proj[0].y, proj[1].x, proj[1].y); // Bottom face, so this is a v

        SDL_RenderDrawLine(renderer, proj[1].x, proj[1].y, proj[2].x, proj[2].y); // Bottom face
        SDL_RenderDrawLine(renderer, proj[2].x, proj[2].y, proj[3].x, proj[3].y); // Bottom face
        SDL_RenderDrawLine(renderer, proj[3].x, proj[3].y, proj[0].x, proj[0].y); // Bottom face
        SDL_RenderDrawLine(renderer, proj[4].x, proj[4].y, proj[5].x, proj[5].y); // Top face
        SDL_RenderDrawLine(renderer, proj[5].x, proj[5].y, proj[6].x, proj[6].y); // Top face
        SDL_RenderDrawLine(renderer, proj[6].x, proj[6].y, proj[7].x, proj[7].y); // Top face
        SDL_RenderDrawLine(renderer, proj[7].x, proj[7].y, proj[4].x, proj[4].y); // Top face
        SDL_RenderDrawLine(renderer, proj[0].x, proj[0].y, proj[4].x, proj[4].y); // Connecting edges
        SDL_RenderDrawLine(renderer, proj[1].x, proj[1].y, proj[5].x, proj[5].y); // Connecting edges
        SDL_RenderDrawLine(renderer, proj[2].x, proj[2].y, proj[6].x, proj[6].y); // Connecting edges
        SDL_RenderDrawLine(renderer, proj[3].x, proj[3].y, proj[7].x, proj[7].y); // Connecting edges

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
