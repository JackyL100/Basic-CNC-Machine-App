#include <utility>
#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <string_view>
#include <unistd.h>
#include "SerialPort.cpp"
#include "TypeAbbreviations.hpp"

#pragma once

// base class for each line in the program

typedef std::pair<int, int> point;

// define each type of line
enum class Mode {
    StraightLine,
    Curve,
    Free
};

void error(const char* msg, int sockfd) {
    perror(msg);
    close(sockfd);
    exit(0);
}

class Movement {
    private:
    public: 
        // start and end points of a line
        point start_point;
        point end_point;

        // used for when creating a line
        static point startLineCoor;
        static point endLineCoor;

        // basic constructor
        Movement(double x1, double y1, double x2, double y2) {
            start_point = std::make_pair(x1, y1);
            end_point = std::make_pair(x2, y2);
        }

        // render the line on the renderer
        // child classes should override this to match their line shape
        virtual void render(SDL_Renderer* target) {
            SDL_RenderDrawLine(target, start_point.first, start_point.second, end_point.first, end_point.second);
        }   

        // turn the string into a string for easy transmission
        // child classes should override this to include their other members
        virtual std::string getCompInfo() const {
            return "L " + std::to_string(start_point.first) + " " + std::to_string(start_point.second) + " " + std::to_string(end_point.first) + " " + std::to_string(end_point.second);
        }

        // send each movement to the ESP32
        static void compile(const std::vector<Movement*>& movements, int& sockfd, bool online) {
            // send initial instructions
            for (auto& m : movements) {
                char buf[256];
                static point currentPos;
                std::string s = m->getCompInfo();
                if (online) {
                    if (recv(sockfd, buf, 256, 0) < 0) error("Error recving", sockfd);
                    if (std::string_view(buf) == "ready") {
                        memset(buf, 0, 256);
                        send(sockfd, s.c_str(), s.size(), 0);
                    } 
                } else {
                    
                }
            }
        }
};

point Movement::startLineCoor = std::make_pair(0,0);
point Movement::endLineCoor = std::make_pair(0,0);