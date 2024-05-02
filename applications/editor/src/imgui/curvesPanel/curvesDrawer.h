#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

class CurvesDrawer
{
public:
    CurvesDrawer() = default;
    ~CurvesDrawer() = default;

    //TODO : J'suis arrivée à ce DrawControlPoint, on dirait que ça veut marcher, les points ont l'air de vouloir bouger mais ils se reset immédiatement
    // j'suis trop fatiguée pour continuer dessus lol

    static void DrawControlPoints(ImDrawList* draw_list, glm::vec3 points[5], float radius, ImU32 col)
    {
        ImVec2 mousePos = ImGui::GetMousePos();
        static bool isDragging = false;
        static int draggedPointIndex = -1;

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            for (int i = 0; i < 5; ++i)
            {
                ImVec2 pointPos = ImVec2(points[i].x, points[i].y);
                if (IsPointHovered(pointPos, mousePos))
                {
                    isDragging = true;
                    draggedPointIndex = i;
                    break;
                }
            }
        }

        if (isDragging)
        {
            ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            points[draggedPointIndex].x += delta.x;
            points[draggedPointIndex].y += delta.y;
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);

            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                isDragging = false;
                draggedPointIndex = -1;
            }
        }

        for (int i = 0; i < 5; ++i)
        {
            ImVec2 pointPos = ImVec2(points[i].x, points[i].y);
            draw_list->AddCircleFilled(pointPos, radius, IsPointHovered(pointPos, mousePos) ? IM_COL32(255, 0, 0, 255) : col);
        }
    }

    static bool IsPointHovered(const ImVec2& pointPos, const ImVec2& mousePos, float radius = 5.0f)
    {
        float dx = mousePos.x - pointPos.x;
        float dy = mousePos.y - pointPos.y;
        float distanceSquared = dx * dx + dy * dy;
        return distanceSquared <= (radius * radius);
    }

    static void DrawBezierSpline(ImDrawList* draw_list, glm::vec3 points[5], ImU32 col, float thickness)
    {
        glm::vec3 lastPoint = EvaluateBezier(points[0], points[1], points[2], points[3], points[4], 0.0f);
        for (float t = 0.01f; t <= 1.0f; t += 0.01f)
        {
            glm::vec3 point = EvaluateBezier(points[0], points[1], points[2], points[3], points[4], t);
            draw_list->AddLine(ImVec2(lastPoint.x, lastPoint.y), ImVec2(point.x, point.y), col, thickness);
            lastPoint = point;
        }
        DrawControlPoints(draw_list, points, 5.0f, IM_COL32(0, 255, 0, 255));
    }

    void DrawCoonsSurface(ImDrawList* draw_list, const glm::vec3 corners[4], ImU32 col)
    {
        glm::vec3 center = (corners[0] + corners[1] + corners[2] + corners[3]) * 0.25f;

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
            rotation.y += delta.x * 0.005f;
            rotation.x += delta.y * 0.005f;
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
        }

        glm::vec3 points[101][101];
        for (int u = 0; u <= 100; ++u)
        {
            for (int v = 0; v <= 100; ++v)
            {
                points[u][v] = EvaluateCoons(corners, u / 100.0f, v / 100.0f);
                points[u][v] = RotatePoint(points[u][v], rotation, center);
            }
        }

        for (int u = 0; u < 100; ++u)
        {
            for (int v = 0; v < 100; ++v)
            {
                if (u < 80 && v < 99)
                {
                    // Stop one step before the right and bottom edges
                    draw_list->AddQuadFilled({points[u][v].x, points[u][v].y}, {points[u][v + 1].x, points[u][v + 1].y}, {points[u + 1][v + 1].x, points[u + 1][v + 1].y}, {points[u + 1][v].x, points[u + 1][v].y}, col);
                }
            }
        }
    }

private:
    glm::vec3 RotatePoint(const glm::vec3& point, const glm::vec3& rotation, const glm::vec3& center)
    {

        glm::vec3 translatedPoint = point - center;


        float cosThetaX = cos(rotation.x);
        float sinThetaX = sin(rotation.x);
        float cosThetaY = cos(rotation.y);
        float sinThetaY = sin(rotation.y);

        glm::vec3 rotatedPoint(
                cosThetaY * translatedPoint.x - sinThetaY * translatedPoint.z,
                sinThetaX * (cosThetaY * translatedPoint.z + sinThetaY * translatedPoint.x) + cosThetaX * translatedPoint.y,
                cosThetaX * (cosThetaY * translatedPoint.z + sinThetaY * translatedPoint.x) - sinThetaX * translatedPoint.y);


        return rotatedPoint + center;
    }

    static glm::vec3 EvaluateBezier(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4, float t)
    {
        float u = 1.0f - t;
        float tt = t * t;
        float ttt = tt * t;
        float tttt = ttt * t;
        float uu = u * u;
        float uuu = uu * u;
        float uuuu = uuu * u;

        return p0 * uuuu +
               p1 * 4.0f * uuu * t +
               p2 * 6.0f * uu * tt +
               p3 * 4.0f * u * ttt +
               p4 * tttt;
    }

    static glm::vec3 EvaluateCoons(const glm::vec3 corners[4], float u, float v)
    {
        glm::vec3 p00 = corners[0];
        glm::vec3 p10 = corners[1];
        glm::vec3 p01 = corners[3];
        glm::vec3 p11 = corners[2];

        // Compute the bilinear interpolation terms
        glm::vec3 p0u = (1.0f - u) * p00 + u * p10;
        glm::vec3 p1u = (1.0f - u) * p01 + u * p11;
        glm::vec3 pv0 = (1.0f - v) * p00 + v * p01;
        glm::vec3 pv1 = (1.0f - v) * p10 + v * p11;

        // Define wave parameters
        float Ax = 80.1f;// Amplitude for x-axis waves
        float Ay = 80.1f;// Amplitude for y-axis waves
        float fx = 1.0f; // Frequency for x-axis waves
        float fy = 1.0f; // Frequency for y-axis waves

        // Compute the Coons patch
        glm::vec3 puv = (1.0f - v) * p0u + v * p1u + (1.0f - u) * pv0 + u * pv1 - ((1.0f - u) * (1.0f - v) * p00 + u * (1.0f - v) * p10 + (1.0f - u) * v * p01 + u * v * p11);

        // Apply wave effect
        puv.x += Ax * sin(fx * 2.0f * glm::pi<float>() * u) * cos(fx * 2.0f * glm::pi<float>() * v);
        puv.y += Ay * sin(fy * 2.0f * glm::pi<float>() * v) * cos(fy * 2.0f * glm::pi<float>() * u);

        return puv;
    }

    glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
};
