#include "movement.hpp"
#include <vector>
#include <cmath>
#pragma once

class CurvedMovement : public Movement {
    private:
        bool horiz;
        int a;
        int b;
        int h;
        std::vector<point> points;
    public:
        CurvedMovement(int x1, int y1, int x2, int y2, int h_) : Movement(x1, y1, x2, y2)  {
            horiz = y1==y2 ? true : false;
            if (horiz) {
                points.reserve(x2-x1);
                b = h_;
                a = (x2 - x1) / 2;
                for (int i = x1; i <= x2; i++) {
                    int y = b * -1 * (sqrt(pow(a,2) - pow(i - (x1 + a), 2))) / a;
                    std::cout << y << "\n";
                    point p = std::make_pair(i, y + y1);
                    std::cout << p.first << "|" << p.second << "\n";
                    points.push_back(std::move(p));
                    //std::cout << points[i].first << "|" <<  points[i].second << "\n";
                }
            } else {
                points.reserve(y2 - y1);
                b = h_;
                a = (y2 - y1) / 2;
                for (int i = y1; i <= y2; i++) {
                    int x = b * (sqrt(pow(a,2) - pow(i - (y1 + a), 2))) / a;
                    point p = std::make_pair(x + x1, i);
                    std::cout << p.first << "|" << p.second << "\n";
                    points.push_back(std::move(p));
                }
            }
            start_point = points[0];
            end_point = points[points.size() - 1];
            h = h_;
        } 
        void render(SDL_Renderer* target) override {
            for (auto& p : points) {
                SDL_RenderDrawPoint(target, p.first, p.second);
            }
        }
        virtual std::string getCompInfo() const override {
            return "C " + std::to_string(start_point.first) + " " + std::to_string(start_point.second) + " " + std::to_string(end_point.first) + " " + std::to_string(end_point.second) + " " + std::to_string(h);
        }
};