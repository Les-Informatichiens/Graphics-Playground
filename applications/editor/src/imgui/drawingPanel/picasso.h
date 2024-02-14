//
// Created by Jean on 2/14/2024.
//

#pragma once


#include "shape.h"

class picasso {

public:
    picasso() : currentShape(Shapes::None) {};
    ~picasso() = default;

    enum class Shapes {
        Circle,
        Square,
        Rectangle,
        Triangle,
        Line,
        None
    };

    void draw(ImDrawList *pList) {

        ImGui::Text("Here you can draw all kind of things!");
        ImGui::ColorEdit4("Background Color", (float*)&backgroundColor);
        ImGui::Separator();

        ImGui::ColorPicker4("Drawing Color", (float*)&drawColor);
        ImGui::DragFloat("Thickness", &thickness, 0.05f, 1.0f, 8.0f, "%.02f");
        ImGui::Separator();


        if (ImGui::Button("Draw line"))
        {
            this->setShape(picasso::Shapes::Line);
        }
        else if (ImGui::Button("Draw circle"))
        {
            this->setShape(picasso::Shapes::Circle);
        }
        else if (ImGui::Button("Draw square"))
        {
            this->setShape(picasso::Shapes::Square);
        }
        else if (ImGui::Button("Draw rectangle"))
        {
            this->setShape(picasso::Shapes::Rectangle);
        }
        else if (ImGui::Button("Draw triangle"))
        {
            this->setShape(picasso::Shapes::Triangle);
        }
        else if (ImGui::Button("Stop drawing"))
        {
            this->setShape(picasso::Shapes::None);
        }
        else if (ImGui::Button("Clear shapes"))
        {
            this->clearShapes();
        }
        else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && this->getShape() != picasso::Shapes::None)
        {
            this->addShape(ImGui::GetMousePos());
        }
        else if (this->getShape() != picasso::Shapes::None && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            this->setMousePos(ImGui::GetMousePos());
        }
        else if (this->getShape() != picasso::Shapes::None && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            this->addShape(ImGui::GetMousePos());
        }

        pList->AddRectFilled({0,0},{1920,1080}, backgroundColor, 0.0f, 0);
        for (auto& shape : shapes) {
            shape->draw(pList);
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && this->getShape() != picasso::Shapes::None){
            shapes.pop_back();
        }
    };

    void addShape(ImVec2 finalPos)
    {

        switch (currentShape) {

            case Shapes::Circle: {
                ImVec2 center = ImVec2((mousePos.x + finalPos.x) / 2, (mousePos.y + finalPos.y) / 2);
                float radius = glm::sqrt(glm::pow(finalPos.x - center.x, 2) + glm::pow(finalPos.y - center.y, 2));
                shapes.emplace_back(new Circle(center, radius, drawColor, 50, thickness));
                break;
            }
            case Shapes::Square:
                shapes.emplace_back(new Square(mousePos, finalPos, drawColor, thickness));
                break;
            case Shapes::Rectangle:
                shapes.emplace_back(new Rectangle(mousePos, finalPos, drawColor, 0.0f, 0, thickness));
                break;
            case Shapes::Triangle: {
                ImVec2 c = ImVec2((mousePos.x + finalPos.x) / 2, finalPos.y);
                shapes.emplace_back(new Triangle(mousePos, finalPos, c,
                                                 drawColor, thickness));
                break;
            }
            case Shapes::Line:
                shapes.emplace_back(new Line(mousePos, finalPos, drawColor, thickness));
                break;
            case Shapes::None:
                break;
        }
    };

    void clearShapes(){
        shapes.clear();
    };

    void setShape(Shapes shape) {
        currentShape = shape;
    };

    Shapes getShape() {
        return currentShape;
    };

    void setMousePos(ImVec2 pos) {
        mousePos = pos;
    };


private:
    ImColor backgroundColor = ImColor(255, 255, 255, 150);
    ImColor drawColor = ImColor(255, 255, 255, 255);
    ImVec2 mousePos = ImVec2(0, 0);
    Shapes currentShape;
    std::vector<Shape*> shapes = {};
    float thickness = 1.0f;

};
