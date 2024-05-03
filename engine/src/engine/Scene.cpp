//
// Created by Jonathan Richard on 2024-03-04.
//

#include "engine/Scene.h"
#include "engine/EntityView.h"
#include "engine/SceneNode.h"
#include "engine/SceneRenderData.h"
#include "engine/components/CameraComponent.h"
#include "engine/components/LightComponent.h"
#include "engine/components/MeshComponent.h"


Scene::Scene()
    : registry(), entityMap()
{
    registry.on_construct<SceneNode>().connect<&Scene::linkSceneNodeWithEntity>();
    registry.on_update<SceneNode>().connect<&Scene::linkSceneNodeWithEntity>();
}

Scene::~Scene() = default;

void Scene::update(float dt)
{
    registry.view<SceneNode>().each([dt](SceneNode& node){
        if (!node.hasParent())
        {
            node.update(dt);
        }
    });
}

void Scene::getSceneRenderData(SceneRenderData& sceneRenderData) const
{
    sceneRenderData.reset();

    // iterate over all entities with a SceneNode and MeshComponent
    registry.view<SceneNode, MeshComponent>().each([&sceneRenderData](const SceneNode& node, const MeshComponent& mesh){
        if (!node.isVisible())
            return;

        MeshRenderData meshRenderData;
        meshRenderData.modelMatrix = node.getWorldTransform().getModel();
        meshRenderData.mesh = mesh.getMesh();
        meshRenderData.material = mesh.getMaterial();
        sceneRenderData.meshRenderData.push_back(meshRenderData);

        if (node.showBoundingBox)
        {
            auto model = node.getWorldTransform().getModel();
            Bounds bounds = mesh.getMesh()->getMesh().bounds;

            // create line segments for each edge of the bounding box, we can join the segments from the top and bottom of the box
            glm::vec3 min = bounds.getMin();
            glm::vec3 max = bounds.getMax();
            glm::vec3 size = bounds.getSize();

            std::vector<glm::vec3> points = {
                    min,
                    min + glm::vec3(size.x, 0.0f, 0.0f),
                    min + glm::vec3(size.x, size.y, 0.0f),
                    min + glm::vec3(0.0f, size.y, 0.0f),
                    min,
                    min + glm::vec3(0.0f, 0.0f, size.z),
                    min + glm::vec3(size.x, 0.0f, size.z),
                    min + glm::vec3(size.x, size.y, size.z),
                    min + glm::vec3(0.0f, size.y, size.z),
                    min + glm::vec3(0.0f, 0.0f, size.z),
                    min + glm::vec3(size.x, 0.0f, size.z),
                    min + glm::vec3(size.x, 0.0f, 0.0f),
                    min + glm::vec3(size.x, size.y, 0.0f),
                    min + glm::vec3(size.x, size.y, size.z),
                    min + glm::vec3(0.0f, size.y, size.z),
                    min + glm::vec3(0.0f, size.y, 0.0f)};

            // split the points into lines
            for (size_t i = 0; i < points.size() - 1; i++)
            {
                sceneRenderData.lineRenderData.push_back({{model * glm::vec4(points[i], 1.0f), model * glm::vec4(points[i + 1], 1.0f)}, {1.0f, 0.0f, 0.0f, 1.0f}});
            }
        }
    });

    // iterate over all entities with a SceneNode and LightComponent
    registry.view<SceneNode, LightComponent>().each([&sceneRenderData](const SceneNode& node, const LightComponent& light){
        if (!node.isVisible())
            return;

        LightData lightData;
        lightData.modelMatrix = node.getWorldTransform().getModel();
        lightData.position = node.getWorldTransform().getPosition();
        lightData.direction = node.getWorldTransform().getForward();
        lightData.light = light.getLight();
        sceneRenderData.lights.push_back(lightData);
    });


    // draw a circle with the lines for testing
    {
        int numPoints = 100;
        float pi = glm::pi<float>();
        float radius = 100.0f;
        glm::vec4 color = {0.0f, 1.0f, 0.0f, 1.0f};
        for (int i = 0; i < numPoints; i++)
        {
            float angle = 2.0f * pi * i / numPoints;
            glm::vec3 p1 = glm::vec3(radius * cos(angle), 0.0f, radius * sin(angle));
            glm::vec3 p2 = glm::vec3(radius * cos(angle + 2.0f * pi / numPoints), 0.0f, radius * sin(angle + 2.0f * pi / numPoints));
            //hueshift color
            color = {color[1], color[2], color[0], color[3]};
            sceneRenderData.lineRenderData.push_back({{p1, p2}, color});
        }
    }

    sceneRenderData.skybox = { skyboxTexture };
}

void Scene::linkSceneNodeWithEntity(entt::registry& reg, entt::entity e)
{
    reg.get<SceneNode>(e).entity = {reg, e};
}

EntityView Scene::createEntity(const std::string& name)
{
    util::UUID uuid = util::UUID::generate();

    entityMap.emplace(uuid, EntityData{ name, uuid, registry.create() });

    EntityView entity = {uuid, this};
    entity.addComponent<SceneNode>(EntityView{uuid, this});

    return entity;
}

void Scene::destroyEntity(const EntityView& entity)
{
    registry.destroy(entity.getHandle());
    entityMap.erase(entity.getUUID());
}

void Scene::destroyEntity(util::UUID uuid)
{
    auto it = entityMap.find(uuid);
    if (it != entityMap.end())
    {
        registry.destroy(it->second.entity);
        entityMap.erase(it);
    }
}

std::optional<EntityView> Scene::getEntity(util::UUID uuid)
{
    auto it = entityMap.find(uuid);
    if (it != entityMap.end())
    {
        return { { it->second.uuid, this } };
    }
    return std::nullopt;
}

std::optional<EntityView> Scene::getEntityByName(const std::string& name)
{
    for (auto& [uuid, entityData]: entityMap)
    {
        if (entityData.name == name)
        {
            return { { uuid, this } };
        }
    }
    return std::nullopt;
}

entt::entity Scene::getEntityHandle(util::UUID uuid)
{
    auto it = entityMap.find(uuid);
    if (it != entityMap.end())
    {
        return it->second.entity;
    }
    return entt::null;
}

std::string& Scene::getEntityName(util::UUID uuid)
{
    auto it = entityMap.find(uuid);
    if (it != entityMap.end())
    {
        return it->second.name;
    }
    throw std::runtime_error("Entity not found");
}

std::vector<SceneNode*> Scene::getCameraNodes()
{
    std::vector<SceneNode*> cameraNodes;
    registry.view<SceneNode, CameraComponent>().each([&cameraNodes](SceneNode& node, CameraComponent& camera){
        cameraNodes.push_back(&node);
    });
    return cameraNodes;
}

std::vector<EntityView> Scene::getRootEntities()
{
    std::vector<EntityView> rootEntities;
    registry.view<SceneNode>().each([&rootEntities, this](entt::entity e, SceneNode& node){
        if (!node.hasParent())
        {
            rootEntities.push_back(node.getEntityView());
        }
    });
    return rootEntities;
}

std::optional<RaycastHit> Scene::raycastFirstHit(util::Ray ray, float maxDistance)
{
    std::optional<RaycastHit> hit;
    float closestDistance = maxDistance;

    registry.view<SceneNode, MeshComponent>().each([&ray, &hit, &closestDistance](SceneNode& node, MeshComponent& mesh){
        if (!node.isVisible())
            return;

        Bounds bounds = mesh.getMesh()->getMesh().bounds;
        glm::mat4 model = node.getWorldTransform().getModel();
        glm::mat4 invModel = glm::inverse(model);

        glm::vec3 rayOrigin = glm::vec3(invModel * glm::vec4(ray.getOrigin(), 1.0f));
        glm::vec3 rayDirection = glm::vec3(invModel * glm::vec4(ray.getDirection(), 0.0f));
        util::Ray localRay(rayOrigin, rayDirection);

        if (bounds.intersects(localRay))
        {
            glm::vec3 localIntersectionPoint = bounds.getIntersectionPoint(localRay);
            glm::vec3 intersectionPoint = glm::vec3(model * glm::vec4(localIntersectionPoint, 1.0f));
            float distance = glm::length(intersectionPoint - ray.getOrigin());
            if (distance < closestDistance)
            {
                closestDistance = distance;
                hit = RaycastHit{node.getEntityView().getUUID(), intersectionPoint, glm::vec3()};
            }
        }
    });

    return hit;
}

std::optional<EntityView> Scene::findMainCameraEntity()
{
    std::optional<EntityView> mainCameraEntity;
    registry.view<SceneNode, CameraComponent>().each([&mainCameraEntity](SceneNode& node, CameraComponent& camera){
        if (camera.isRenderingToScreen())
        {
            mainCameraEntity = node.getEntityView();
        }
    });
    return mainCameraEntity;
}
