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

    Line(ImVec2 start, ImVec2 end, ImU32 col, float thickness) : start(start), end(end), col(col),
                                                                 thickness(thickness) {};

    void draw(ImDrawList *pList) const override {
        pList->AddLine(start, end, col, thickness);
    }
};

struct Circle : public Shape {
    ImVec2 center;
    float radius;
    ImU32 col;
    int numSegments;
    float thickness;

    Circle(ImVec2 center, float radius, ImU32 col, int numSegments, float thickness) : center(center), radius(radius),
                                                                                       col(col),
                                                                                       numSegments(numSegments),
                                                                                       thickness(thickness) {};

    void draw(ImDrawList *pList) const override {
        pList->AddCircle(center, radius, col, numSegments, thickness);
    }
};

struct Square : public Shape {
    ImVec2 topLeft;
    ImVec2 bottomRight;
    ImU32 col;
    float thickness;

    Square(ImVec2 topLeft, ImVec2 bottomRight, ImU32 col, float thickness) : topLeft(topLeft), bottomRight(bottomRight),
                                                                             col(col), thickness(thickness) {};

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

    Rectangle(ImVec2 topLeft, ImVec2 bottomRight, ImU32 col, float rounding, int roundingCorners, float thickness)
            : topLeft(topLeft), bottomRight(bottomRight), col(col), rounding(rounding),
              roundingCorners(roundingCorners), thickness(thickness) {};

    void draw(ImDrawList *pList) const override {
        pList->AddRect(topLeft, bottomRight, col, rounding, roundingCorners, thickness);
    }
};

struct Triangle : public Shape {
    ImVec2 a, b, c;
    ImU32 col;
    float thickness;

    Triangle(ImVec2 a, ImVec2 b, ImVec2 c, ImU32 col, float thickness) : a(a), b(b), c(c), col(col),
                                                                         thickness(thickness) {};

    void draw(ImDrawList *pList) const override {
        pList->AddTriangle(a, b, c, col, thickness);
    }
};

struct L : public Shape {
    ImVec2 topLeft;
    ImVec2 bottomRight;
    ImU32 col;

    L(ImVec2 topLeft, ImVec2 bottomRight, ImU32 col) : topLeft(topLeft), bottomRight(bottomRight), col(col) {};

    void draw(ImDrawList *pList) const override {
        pList->AddRectFilled(topLeft, ImVec2(bottomRight.x - (bottomRight.x - topLeft.x) / 2,
                                             topLeft.y + (bottomRight.y - topLeft.y) / 2), col);
        pList->AddRectFilled(ImVec2(topLeft.x, topLeft.y + (bottomRight.y - topLeft.y) / 2), bottomRight, col);
    }

};

struct Tetrahedron : public Shape {
    ImVec2 base1, base2, base3, apex; // Tetrahedron vertices
    ImU32 col;
    float thickness;

    Tetrahedron(ImVec2 base1, ImVec2 base2, ImVec2 base3, ImVec2 apex, ImU32 col, float thickness)
            : base1(base1), base2(base2), base3(base3), apex(apex), col(col), thickness(thickness) {};

    void draw(ImDrawList *pList) const override {
        // Draw base
        pList->AddLine(base1, base2, col, thickness);
        pList->AddLine(base2, base3, col, thickness);
        pList->AddLine(base3, base1, col, thickness);

        // Draw sides
        pList->AddLine(apex, base1, col, thickness);
        pList->AddLine(apex, base2, col, thickness);
        pList->AddLine(apex, base3, col, thickness);
    }
};


