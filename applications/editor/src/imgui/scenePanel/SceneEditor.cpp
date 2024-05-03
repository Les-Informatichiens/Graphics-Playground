//
// Created by Jonathan Richard on 2024-03-08.
//

#include "SceneEditor.h"
#include "engine/components/CameraComponent.h"
#include "engine/components/LightComponent.h"
#include "engine/components/MeshComponent.h"
#include "imgui.h"


void SceneEditor::draw()
{
    drawMaterialEditor();

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
                            ImGui::Text("Vertex count: %zu", mesh.getMesh()->getMesh().vertices.size());
                            ImGui::Text("Index count: %zu", mesh.getMesh()->getMesh().indices.size());
                            ImGui::EndTooltip();
                        }
                        ImGui::Text("Material:");
                        ImGui::SameLine(); ImGui::TextDisabled(mesh.getMaterial() ? mesh.getMaterial()->getName().c_str() : "None");
                        if (ImGui::SameLine(); ImGui::SmallButton("Select"))
                        {
                            // open a material selection context menu
                            ImGui::OpenPopup("Material Selection");
                        }
                        if (ImGui::BeginPopup("Material Selection"))
                        {
                            if (materialResources_.empty())
                            {
                                ImGui::Text("No materials available");
                            }
                            for (auto& [name, material] : materialResources_)
                            {
                                if (ImGui::Selectable(name.c_str()))
                                {
                                    mesh.setMaterial(material.lock());
                                }
                            }
                            ImGui::EndPopup();
                        }

                    }
                }
                if (entity->hasComponent<LightComponent>())
                {
                    auto& light = entity->getComponent<LightComponent>();
                    if (ImGui::CollapsingHeader("Light"))
                    {
                        ImGui::TextDisabled("Light properties");

                        ImGui::Combo("Type", (int*)&light.getLight()->type, "Directional\0Point\0Spot\0");

                        ImGui::ColorEdit3("Color", &light.getLight()->color.r,
                                          ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_AlphaBar);


                        ImGui::DragFloat("Intensity", &light.getLight()->intensity, 0.1f);

                        if (light.getLight()->type != LightType::Directional)
                        {
                            ImGui::DragFloat("Constant", &light.getLight()->constant, 0.1f);
                            ImGui::DragFloat("Linear", &light.getLight()->linear, 0.1f);
                            ImGui::DragFloat("Quadratic", &light.getLight()->quadratic, 0.1f);
                        }

                        if (light.getLight()->type == LightType::Spot)
                        {
                            ImGui::DragFloat("Cutoff", &light.getLight()->cutOff, 0.1f);
                            ImGui::DragFloat("Outer Cutoff", &light.getLight()->outerCutOff, 0.1f);
                        }
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

void SceneEditor::drawMaterialEditor()
{
    ImGui::Begin("Material Editor");

    if (ImGui::Button("Refresh"))
    {
        refreshMaterialResources();
    }

    // material list
    ImGui::BeginGroup();
    ImGui::Text("Material List");
    ImGui::BeginChild("Material List", ImVec2(150, 0), ImGuiChildFlags_Border);

    // material list
    for (auto& [name, material] : materialResources_)
    {
        if (ImGui::Selectable(name.c_str(), selectedMaterial_ == name))
        {
            selectedMaterial_ = name;
        }
    }

    ImGui::EndChild();
    ImGui::EndGroup();
    ImGui::SameLine();

    // material property editor
    ImGui::BeginGroup();
    ImGui::Text("Material Properties");
    ImGui::BeginChild("Material Properties", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_Border);

    if (!selectedMaterial_.empty())
    {
        struct alignas(16) PBRSettings {
            // Flags
            int useAlbedoMap = true;
            int useNormalMap = true;
            int useMetallicMap = true;
            int useRoughnessMap = true;
            int useAOMap = true;
            int useEmissiveMap = true;

            // Data
            alignas(16) glm::vec3 baseColor = glm::vec3(1.0f);
            float metallic = 0.0f;
            float roughness = 0.5f;
            float ao = 1.0f;
            alignas(16) glm::vec3 emissionColor = glm::vec3(1.0f);
            float emissionIntensity = 1.0f;
            int lightModel = 0;
        };

        if (auto it = materialResources_.find(selectedMaterial_); it != materialResources_.end())
        {
            if (auto mat1 = it->second.lock())
            {
                bool hasEdited = false;
                ImGui::SeparatorText(it->first.c_str());
                PBRSettings pbrSettings;
                mat1->getCachedUniformData("Settings", pbrSettings);
                hasEdited |= ImGui::Checkbox("Use Albedo Map", reinterpret_cast<bool*>(&pbrSettings.useAlbedoMap));
                hasEdited |= ImGui::Checkbox("Use Normal Map", reinterpret_cast<bool*>(&pbrSettings.useNormalMap));
                hasEdited |= ImGui::Checkbox("Use Metallic Map", reinterpret_cast<bool*>(&pbrSettings.useMetallicMap));
                hasEdited |= ImGui::Checkbox("Use Roughness Map", reinterpret_cast<bool*>(&pbrSettings.useRoughnessMap));
                hasEdited |= ImGui::Checkbox("Use AO Map", reinterpret_cast<bool*>(&pbrSettings.useAOMap));
                hasEdited |= ImGui::Checkbox("Use Emissive Map", reinterpret_cast<bool*>(&pbrSettings.useEmissiveMap));
                hasEdited |= ImGui::ColorEdit3("Base Color", &pbrSettings.baseColor.x);
                hasEdited |= ImGui::SliderFloat("Metallic", &pbrSettings.metallic, 0.0f, 1.0f);
                hasEdited |= ImGui::SliderFloat("Roughness", &pbrSettings.roughness, 0.0f, 1.0f);
                hasEdited |= ImGui::SliderFloat("AO", &pbrSettings.ao, 0.0f, 1.0f);
                hasEdited |= ImGui::ColorEdit3("Emission Color", &pbrSettings.emissionColor.x);
                hasEdited |= ImGui::SliderFloat("Emission Intensity", &pbrSettings.emissionIntensity, 0.0f, 10.0f);
                if (hasEdited)
                {
                    mat1->setUniformBuffer("Settings", &pbrSettings, sizeof(pbrSettings), 2);
                }

                // textures
                auto textures = mat1->getAllCachedTextureSamplers();
                for (auto [name, texResource] : textures)
                {
                    if (texResource && texResource->isLoaded())
                    {
                        ImGui::Text(name.c_str());
                        ImGui::SameLine();
                        ImGui::TextDisabled("Texture: %d", texResource->getTexture().get());
                        if (ImGui::ImageButton((ImTextureID)texResource->getTexture().get(), ImVec2(100, 100)))
                        {
                            // open a texture selection context menu
                            ImGui::OpenPopup("Texture Selection");
                            texSamplerForSelection_ = name;
                        }
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::BeginTooltip();
                            ImGui::Image((ImTextureID)texResource->getTexture().get(), ImVec2(256, 256));
                            ImGui::TextDisabled("Texture info");
                            ImGui::Text("Width: %d", texResource->getTexture()->getWidth());
                            ImGui::SameLine(); ImGui::Text("Height: %d", texResource->getTexture()->getHeight());
                            ImGui::Text("Format: %d", texResource->getTexture()->getFormat());
                            ImGui::Text("Type: %d", texResource->getTexture()->getType());

                            ImGui::EndTooltip();
                        }
                    }
                }
                if (ImGui::BeginPopup("Texture Selection"))
                {
                    for (auto& [texName, tex] : engineInstance_.resourceManager.getTextures())
                    {
                        if (ImGui::Selectable(texName.c_str()))
                        {
                            mat1->setTextureSampler(texSamplerForSelection_, tex, 0);
                            texSamplerForSelection_ = "";
                        }
                    }
                    ImGui::EndPopup();
                }
            }
        }
    }

    ImGui::EndChild();
    ImGui::EndGroup();
    ImGui::End();
}

void SceneEditor::refreshMaterialResources()
{
    materialResources_.clear();
    std::vector<std::string> materials = {"pbrMaterial", "pbrMaterial1", "pbrMaterial2"};
    for (auto& [name, mat] : engineInstance_.resourceManager.getMaterials())
    {
        auto material = mat;
        if (material)
        {
            materialResources_[name] = material;
        }
    }
    selectedMaterial_ = "";
    if (!materialResources_.empty())
    {
        selectedMaterial_ = materialResources_.begin()->first;
    }
}

void SceneEditor::clearSelection()
{
    selectedEntities_.clear();
    lastSelectedEntity_ = util::UUID::zero();
}
void SceneEditor::setEntitySelection(util::UUID uuid, bool isSelected)
{
    selectedEntities_[uuid] = isSelected;
    lastSelectedEntity_ = uuid;
}

bool SceneEditor::isEntitySelected(util::UUID uuid) const
{
    return selectedEntities_.find(uuid) != selectedEntities_.end();
}
