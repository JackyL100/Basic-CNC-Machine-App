/*
Notes:
only straight lines with mouse
curved lines must use terminal
deleting lines must use terminal
*/

// Jacky Lei
// MINI CNC MACHINE PROJECT
// Creates an application that allows me to draw a basic 2D design and send it to my machine to draw

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "movement.hpp"
#include "curved_movement.hpp"
#include <vector>
#include <string>
#include <utility>
//g++ main.cpp -L /usr/local/Cellar/sdl2/2.0.20/include -lSDL2 -lSDL2_image -std=c++11

// dimensions of the window
const int WIDTH = 960;
const int HEIGHT = 540;

// global renderer for rendering
SDL_Renderer *renderTarget = nullptr;

// creating texture from a picture
SDL_Texture* createTexture(std::string filePath) {
    SDL_Texture* texture = nullptr;
    SDL_Surface* surface = IMG_Load(filePath.c_str());
    if (surface == NULL) {
        std::cout << "Error loading surface" << SDL_GetError() << std::endl;
    } else {
        texture = SDL_CreateTextureFromSurface(renderTarget, surface);
        if (texture == NULL) {
            std::cout << "Error loading texture" << SDL_GetError() << std::endl;
        }
    }
    SDL_FreeSurface(surface);
    return texture;
}

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Mini CNC", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == NULL) {// exit program if unable to create window
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        return 1;
    }
    // setting up objects for SDL component of the program
    SDL_Event windowEvent;
    SDL_MouseButtonEvent mouseEvent;
    renderTarget = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    const Uint8* keyState;
    SDL_Texture* texture = createTexture("white_bg.png");

    // setting up the connection to the ESP32
    int sockfd, port;
    struct sockaddr_in serv_addr;
    port = 3490;
    std::string ipAddress = "192.168.1.164";
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR OPENING SOCKET", sockfd);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &serv_addr.sin_addr);

    //if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) error("ERROR CONNECTING", sockfd);



    int serialport = openAndConfigureSerialPort("/dev/cu.usbmodem14201", 9600);
    if (serialport < 0) error("ERROR OPENING SERIAL PORT", serialport);

    // vector containing each line so it's easier to send to the machine
    std::vector<Movement*> lines;

    // variables for deciding program behavior
    bool running = true;
    bool creatingLine = false;
    bool online = false;
    Mode lineMode;

    // storing mouse coordinates for drawing lines
    int mouseX;
    int mouseY;

    SDL_SetRenderDrawColor(renderTarget, 255,0,0,255);

    while(running) {
        SDL_Delay(30);
        while(SDL_PollEvent(&windowEvent)!=0) {
            if (SDL_QUIT == windowEvent.type) { // exit if program 'x'ed out
                running = false;
            } else  if (windowEvent.type == SDL_KEYDOWN) {
                switch(windowEvent.key.keysym.sym) { // check what key was pressed
                    case SDLK_s:// use straight line mode
                        lineMode = Mode::StraightLine;
                        std::cout << "Line Mode is now Straight\n";
                        break;
                    case SDLK_c: // use curves
                        int x1, x2, y1, y2, h;
                        std::cout << "Enter x1, y1, x2, y2, h\n";
                        std::cin >> x1 >> y1 >> x2 >> y2 >> h;
                        lines.push_back(new CurvedMovement(x1, y1, x2, y2, h));
                        std::cout << "Lines: " << lines.size() << "\n";
                        break;
                    case SDLK_DELETE: // delete a line
                        std::cout << "Enter num of line to be deleted\n";
                        int d;
                        std::cin >> d;
                        delete lines[d];
                        lines[d] = nullptr;
                        break;
                    case SDLK_e: // sends the drawing to the ESP32
                            if (online) {
                                Movement::compile(lines, sockfd, online);
                            } else {
                                Movement::compile(lines, serialport, online);
                            }
                            
                        break;
                    case SDLK_t: // toggle online status in case user wants to use serial communication instead
                        online = !online;
                        std::cout << "Online status is now: " << online << "\n";
                        break;
                }
            } else if (windowEvent.type == SDL_MOUSEBUTTONDOWN) {
                if (lineMode == Mode::StraightLine) { // handle drawing straight lines
                    creatingLine = !creatingLine;
                    if (creatingLine) {
                        SDL_GetMouseState(&Movement::startLineCoor.first, &Movement::startLineCoor.second);
                    } else {
                        SDL_GetMouseState(&Movement::endLineCoor.first, &Movement::endLineCoor.second);
                        lines.push_back(new Movement(Movement::startLineCoor.first, Movement::startLineCoor.second, Movement::endLineCoor.first, Movement::endLineCoor.second));
                        std::cout << "Lines: " << lines.size() << "\n";
                        creatingLine = false;
                    }
                } 
            } 
        }
        // redrawing the frame
        SDL_RenderClear(renderTarget);
        SDL_SetRenderTarget(renderTarget, texture);
        SDL_RenderCopy(renderTarget, texture, NULL, NULL);
        // draw each completed line
        for (auto& m : lines) {
            m->render(renderTarget);
        }
        // draw the incomplete line
        if (creatingLine) {
            SDL_GetMouseState(&mouseX, &mouseY);
            SDL_RenderDrawLine(renderTarget, Movement::startLineCoor.first,Movement::startLineCoor.second, mouseX, mouseY);
        }
        // display the elements of the frame to the window
        SDL_RenderPresent(renderTarget);
    }
    // clean up SDL components, close connection, clean up pointers
    SDL_DestroyRenderer(renderTarget);
    SDL_DestroyTexture(texture);
    SDL_DestroyWindow(window);
    close(sockfd);
    renderTarget = nullptr;
    texture = nullptr;
    window = nullptr;
    for (auto& m : lines) {
        delete m;
    }
}