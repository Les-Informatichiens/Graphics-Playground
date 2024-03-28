//
// Created by Jonathan Richard on 2024-03-08.
//

#include "SceneEditor.h"
#include "engine/components/CameraComponent.h"
#include "engine/components/MeshComponent.h"
#include "imgui.h"


void SceneEditor::draw()
{
    ImGui::Begin("Scene Hierarchy");

    ImGui::Text("Scene Hierarchy");

    // foldable Tree nodes for the scene hierarchy
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);

    auto& stage = engineInstance_.getStage();
    auto* scene = stage.getScene();
    if (scene)
    {
        if (ImGui::CollapsingHeader("Scene"))
        {
            for (auto& entity : scene->getRootEntities())
            {
                drawSceneTree(entity.getSceneNode());

            }
        }
    }
    else
    {
        ImGui::Text("No scene loaded");
    }

    ImGui::End();

    ImGui::Begin("Properties");
    ImGui::Text("Properties");

    if (!selectedEntities_.empty())
    {
        for (auto& [uuid, isSelected] : selectedEntities_)
        {
            if (isSelected)
            {
                std::optional<EntityView> entity = scene->getEntity(uuid);
                if (!entity)
                {
                    continue;
                }
                ImGui::SeparatorText(entity->getName().c_str());
                auto& transform = entity->getSceneNode().getTransform();
                ImGui::PushID(uuid);
                if (ImGui::CollapsingHeader("Transform"))
                {
                    // rotation in euler angles and then convert to quaternion
                    glm::vec3 eulerAngles = glm::degrees(glm::eulerAngles(transform.rotation_));


                    ImGui::DragFloat3("Position", &transform.position_.x, 0.1f);
                    bool rotationModified = ImGui::DragFloat3("Rotation", &eulerAngles.x, 0.1f);
                    if (rotationModified)
                    {
                        transform.rotation_ = glm::quat(glm::radians(eulerAngles));
                    }
                    ImGui::DragFloat3("Scale", &transform.scale_.x, 0.1f);
                    ImGui::Checkbox("Visible", &entity->getSceneNode().visible);
                }
                if (entity->hasComponent<CameraComponent>())
                {
                    auto& camera = entity->getComponent<CameraComponent>();
                    if (ImGui::CollapsingHeader("Camera"))
                    {
                        ImGui::TextDisabled("Camera properties");
                        bool cameraModified = false;
                        if (camera.getCamera()->projectionType == Camera::ProjectionType::Perspective)
                        {
                            cameraModified |= ImGui::DragFloat("FOV", &camera.getCamera()->fov, 0.1f);
                        }
                        else
                        {
                            cameraModified |= ImGui::DragFloat("Ortho Size", &camera.getCamera()->orthoSize_, 0.1f);
                        }
                        cameraModified |= ImGui::DragFloat("Near", &camera.getCamera()->nearClip, 0.1f);
                        cameraModified |= ImGui::DragFloat("Far", &camera.getCamera()->farClip, 0.1f);
                        cameraModified |= ImGui::DragFloat("Aspect Ratio", &camera.getCamera()->aspectRatio, 0.1f);
                        cameraModified |= ImGui::Combo("Projection Type", (int*)&camera.getCamera()->projectionType, "Perspective\0Orthographic\0");

                        ImGui::TextDisabled("Render Target");
                        auto& renderTarget = camera.renderTarget;
                        // add a minimized color picker for the clear color, it pops up a color picker when clicked
                        // viewport
                        ImGui::ColorEdit4("Clear Color", &renderTarget.clearColor.r, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_AlphaBar);
                        ImGui::SameLine(); ImGui::Checkbox("Clear", &renderTarget.clear);
                        if (renderTarget.colorTexture)
                        {
                            ImGui::Text("Render texture: %d", renderTarget.colorTexture.get());
                            ImGui::ImageButton((ImTextureID)renderTarget.colorTexture.get(), ImVec2(100, 100));
                            if (ImGui::IsItemHovered())
                            {
                                ImGui::BeginTooltip();
                                ImGui::Image((ImTextureID)renderTarget.colorTexture.get(), ImVec2(256, 256));
                                ImGui::TextDisabled("Texture info");
                                ImGui::Text("Width: %d", renderTarget.colorTexture->getWidth());
                                ImGui::SameLine(); ImGui::Text("Height: %d", renderTarget.colorTexture->getHeight());
                                ImGui::Text("Format: %d", renderTarget.colorTexture->getFormat());
                                ImGui::Text("Type: %d", renderTarget.colorTexture->getType());

                                ImGui::EndTooltip();
                            }
                        }
                        else
                        {
                            ImGui::Text("No render texture attached");
                        }


                        if (cameraModified)
                        {
                            camera.getCamera()->updateProjectionMatrix();
                        }
                    }
                }
                if (entity->hasComponent<MeshComponent>())
                {
                    auto& mesh = entity->getComponent<MeshComponent>();
                    if (ImGui::CollapsingHeader("Mesh"))
                    {
                        ImGui::TextDisabled("Mesh properties");
                        ImGui::Text("Mesh: %d", mesh.getMesh().get());
                        if (ImGui::IsItemHovered())
                        {
                            // mesh info
                            ImGui::BeginTooltip();
                            ImGui::TextDisabled("Mesh info");
                            ImGui::Text("Vertex count: %zu", mesh.getMesh()->vertices.size());
                            ImGui::Text("Index count: %zu", mesh.getMesh()->indices.size());
                            ImGui::EndTooltip();
                        }
                        ImGui::Text("Material: %d", mesh.getMaterial().get());
                    }
                }
                ImGui::PopID();
            }
        }
    }
    else
    {
        ImGui::Text("No entity selected");
    }
    ImGui::End();


}

void SceneEditor::drawSceneTree(SceneNode& node)
{
    ImGui::PushID(&node);
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);

    bool isNodeClicked = false;
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;
    if (selectedEntities_.find(node.getEntityView().getUUID()) != selectedEntities_.end())
    {
        flags |= ImGuiTreeNodeFlags_Selected;
        node.setShowBoundingBox(true);
    }
    else
    {
        node.setShowBoundingBox(false);
    }

    if (node.getChildren().empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        ImGui::TreeNodeEx(node.getName().c_str(), flags);
        isNodeClicked = ImGui::IsItemClicked();
    }
    else
    {
        flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

        // double click to open, single to select
        bool open = ImGui::TreeNodeEx(node.getName().c_str(), flags);
        isNodeClicked = ImGui::IsItemClicked();
        if (open)
        {
            for (auto& child : node.getChildren())
            {
                drawSceneTree(*child);
            }
            ImGui::TreePop();
        }
    }

    // single click to select, even if the node is not open
    if (isNodeClicked)
    {
        if (ImGui::GetIO().KeyCtrl)
        {
            selectedEntities_[node.getEntityView().getUUID()] = !selectedEntities_[node.getEntityView().getUUID()];
            lastSelectedEntity_ = node.getEntityView().getUUID();
        }
        else
        {
            selectedEntities_.clear();
            selectedEntities_[node.getEntityView().getUUID()] = true;
            lastSelectedEntity_ = node.getEntityView().getUUID();
        }
    }


    ImGui::PopID();
}
