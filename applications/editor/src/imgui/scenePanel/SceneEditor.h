//
// Created by Jonathan Richard on 2024-03-08.
//

#pragma once

#include "engine/EngineInstance.h"

class SceneEditor
{
public:
    explicit SceneEditor(EngineInstance& engineInstance) : engineInstance_(engineInstance) {}

    void drawSceneTree(SceneNode& node);
    void draw();
    void drawMaterialEditor();
    void refreshMaterialResources();

    void setEntitySelection(util::UUID uuid, bool isSelected);
    bool isEntitySelected(util::UUID uuid) const;
    void clearSelection();
    auto getSelectedEntities() const { return selectedEntities_; }
    std::pair<bool, util::UUID> getLastSelectedEntity() const { return {!selectedEntities_.empty(), lastSelectedEntity_}; }

private:

    EngineInstance& engineInstance_;
    std::unordered_map<util::UUID, bool> selectedEntities_;
    util::UUID lastSelectedEntity_;

    std::string texSamplerForSelection_;
    std::unordered_map<std::string, std::weak_ptr<MaterialResource>> materialResources_;
    std::string selectedMaterial_;
};