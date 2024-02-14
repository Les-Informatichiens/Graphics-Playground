//
// Created by Jean on 2/14/2024.
//

#pragma once

#include "imgui.h"
#include <vector>
#include <glm/glm.hpp>
class Shape {
public:
    virtual void draw(ImDrawList *pList) const = 0;
};

struct Line : public Shape {
    ImVec2 start, end;
    ImU32 col;
    float thickness;

    Line(ImVec2 start, ImVec2 end, ImU32 col, float thickness) : start(start), end(end), col(col), thickness(thickness) {};

    void draw(ImDrawList *pList) const override {
        pList->AddLine(start, end, col, thickness);
    }
};

struct Circle : public Shape{
    ImVec2 center;
    float radius;
    ImU32 col;
    int numSegments;
    float thickness;

    Circle(ImVec2 center, float radius, ImU32 col, int numSegments, float thickness) : center(center), radius(radius), col(col), numSegments(numSegments), thickness(thickness) {};

    void draw(ImDrawList *pList) const override {
        pList->AddCircle(center, radius, col, numSegments, thickness);
    }
};

struct Square : public Shape{
    ImVec2 topLeft;
    ImVec2 bottomRight;
    ImU32 col;
    float thickness;

    Square(ImVec2 topLeft, ImVec2 bottomRight, ImU32 col, float thickness) : topLeft(topLeft), bottomRight(bottomRight), col(col), thickness(thickness) {};

    void draw(ImDrawList *pList) const override {
        pList->AddRect(topLeft, bottomRight, col, 0.0f, 0, thickness);
    }
};

struct Rectangle : public Shape {
    ImVec2 topLeft;
    ImVec2 bottomRight;
    ImU32 col;
    float rounding;
    int roundingCorners;
    float thickness;

    Rectangle(ImVec2 topLeft, ImVec2 bottomRight, ImU32 col, float rounding, int roundingCorners, float thickness) : topLeft(topLeft), bottomRight(bottomRight), col(col), rounding(rounding), roundingCorners(roundingCorners), thickness(thickness) {};

    void draw(ImDrawList *pList) const override {
        pList->AddRect(topLeft, bottomRight, col, rounding, roundingCorners, thickness);
    }
};

struct Triangle : public Shape{
    ImVec2 a, b, c;
    ImU32 col;
    float thickness;

    Triangle(ImVec2 a, ImVec2 b, ImVec2 c, ImU32 col, float thickness) : a(a), b(b), c(c), col(col), thickness(thickness) {};

    void draw(ImDrawList *pList) const override {
        pList->AddTriangle(a, b, c, col, thickness);
    }
};
struct L : public Shape{
    ImVec2 a, b, c;
    ImU32 col;
    float thickness;

    L(ImVec2 a, ImVec2 b, ImVec2 c, ImU32 col, float thickness) : a(a), b(b), c(c), col(col), thickness(thickness) {};

    void draw(ImDrawList *pList) const override {
        ImVec2 c2 = ImVec2(a.x - glm::abs(c.x - a.x), c.y);
        pList->AddTriangle(a, b, c, col, thickness);
        pList->AddTriangle(a, c, c2, col, thickness);
    }
};
struct Sploosh : public Shape{
    ImVec2 center;
    float radius;
    ImU32 col;
    int numSegments;

    Sploosh(ImVec2 center, float radius, ImU32 col, int numSegments) : center(center), radius(radius), col(col), numSegments(numSegments) {};

    void draw(ImDrawList *pList) const override {
        pList->AddCircleFilled(center, radius, col, numSegments);
        pList->AddCircleFilled(center, radius, col, numSegments);
        pList->AddCircleFilled(center, radius, col, numSegments);

    }
};


