//
// Created by Jonathan Richard on 2024-02-09.
//

#include "engine/EngineInstance.h"
#include "engine/EntityView.h"
#include "engine/LineRenderer.h"
#include "engine/MeshRenderer.h"
#include "engine/OBJ_Loader.h"
#include "engine/components/CameraComponent.h"
#include "engine/components/LightComponent.h"
#include "engine/components/MeshComponent.h"
#include <iostream>
#include <utility>

#undef OBJL_CONSOLE_OUTPUT

EngineInstance::EngineInstance(InstanceDesc desc)
    : desc(std::move(desc)), renderer(), resourceManager(), stage(), sceneRenderer(), input()
{
    activeCamera = std::make_unique<Camera>("camera");
}

EngineInstance::~EngineInstance()
{
}

void EngineInstance::updateDisplay(int width, int height)
{
    desc.width = width;
    desc.height = height;

    if (activeCamera)
        activeCamera->setProjectionConfig(90.0f, (float)desc.width / (float)desc.height, 0.1f, 100.0f);
}

void EngineInstance::initialize()
{
    // ----------- Initializing resourceManager ------------

    ResourceManagerDesc resourceManagerDesc;
    resourceManager.initialize(resourceManagerDesc);

    // ----------- Initializing renderer ------------

    graphics::RendererDesc rendererDesc;
    renderer.initialize(rendererDesc);
    testRenderPass.initialize(renderer.getDevice());

    // ----------- Initializing scene objects ------------

    // Setup camera
    activeCamera->setProjectionConfig(90.0f, (float)desc.width / (float)desc.height, 0.1f, 100.0f);
//    activeCamera->getTransform().setPosition({ 0.0f, 0.0f, 20.0f });


    // Create materials

    // PBR material
    {
        auto vs = R"(
            #version 450
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec3 inNormal;
            layout(location = 2) in vec2 inTexCoord;

            layout(location = 0) out vec2 fragTexCoord;
            layout(location = 1) out vec3 fragWorldPos;
            layout(location = 2) out vec3 fragNormal;

            layout(binding = 0) uniform UBO {
                mat4 model;
                mat4 view;
                mat4 proj;
            } ubo;

            void main() {
                gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
                fragTexCoord = inTexCoord;
                fragWorldPos = vec3(ubo.model * vec4(inPosition, 1.0));
                mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
                fragNormal = normalMatrix * inNormal;
            }
        )";

        auto fs = R"(
            #version 450
            layout(location = 0) in vec2 fragTexCoord;
            layout(location = 1) in vec3 fragWorldPos;
            layout(location = 2) in vec3 fragNormal;
            out vec4 fragColor;

            layout(binding = 1) uniform sampler2D albedoMap;
            layout(binding = 2) uniform sampler2D normalMap;
            layout(binding = 3) uniform sampler2D metallicMap;
            layout(binding = 4) uniform sampler2D roughnessMap;
            layout(binding = 5) uniform sampler2D aoMap;

            struct Material {
                vec3 albedo;
                float metallic;
                float roughness;
                float ao;
            };

            struct Light {
                vec3 position;
                vec3 color;
            };

            const int MAX_LIGHTS = 10;
            layout(binding = 6) uniform Lights {
                int lightCount;
                Light lights[MAX_LIGHTS];
            };

            layout(binding = 1) uniform Constants {
                vec3 cameraPos;
                vec3 cameraDir;
                vec3 lightDir;
                float shininess;
            } constants;

            const float PI = 3.14159265359;
            // ----------------------------------------------------------------------------
            float DistributionGGX(vec3 N, vec3 H, float roughness)
            {
                float a = roughness*roughness;
                float a2 = a*a;
                float NdotH = max(dot(N, H), 0.0);
                float NdotH2 = NdotH*NdotH;

                float nom   = a2;
                float denom = (NdotH2 * (a2 - 1.0) + 1.0);
                denom = PI * denom * denom;

                return nom / denom;
            }
            // ----------------------------------------------------------------------------
            float GeometrySchlickGGX(float NdotV, float roughness)
            {
                float r = (roughness + 1.0);
                float k = (r*r) / 8.0;

                float nom   = NdotV;
                float denom = NdotV * (1.0 - k) + k;

                return nom / denom;
            }
            // ----------------------------------------------------------------------------
            float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
            {
                float NdotV = max(dot(N, V), 0.0);
                float NdotL = max(dot(N, L), 0.0);
                float ggx2 = GeometrySchlickGGX(NdotV, roughness);
                float ggx1 = GeometrySchlickGGX(NdotL, roughness);

                return ggx1 * ggx2;
            }
            // ----------------------------------------------------------------------------
            vec3 fresnelSchlick(float cosTheta, vec3 F0)
            {
                return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
            }
            // ----------------------------------------------------------------------------

            void main() {
                vec3 albedo = pow(texture(albedoMap, fragTexCoord).rgb, vec3(2.2));

                vec3 normal = texture(normalMap, fragTexCoord).rgb;
                float metallic = texture(metallicMap, fragTexCoord).r;
                float roughness = texture(roughnessMap, fragTexCoord).r;
                float ao = texture(aoMap, fragTexCoord).r;

                Material material = Material(albedo, metallic, roughness, ao);

                vec3 camPos = constants.cameraPos;

                vec3 N = normalize(fragNormal);
                vec3 V = normalize(camPos - fragWorldPos);

                vec3 F0 = vec3(0.04);
                F0 = mix(F0, albedo, metallic);

                vec3 Lo = vec3(0.0);
                for (int i = 0; i < lightCount; ++i) {
                    vec3 L = normalize(lights[i].position - fragWorldPos);
                    vec3 H = normalize(V + L);
                    float distance = length(lights[i].position - fragWorldPos);
                    float attenuation = 1.0 / (distance * distance);
                    vec3 radiance = lights[i].color * attenuation;

                    float NDF = DistributionGGX(N, H, material.roughness);
                    float G = GeometrySmith(N, V, L, material.roughness);
                    vec3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

                    vec3 numerator = NDF * G * F;
                    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
                    vec3 specular = numerator / denominator;

                    vec3 kS = F;
                    vec3 kD = vec3(1.0) - kS;
                    kD *= 1.0 - material.metallic;

                    float NdotL = max(dot(N, L), 0.0);

                    Lo += (kD * material.albedo / PI + specular) * radiance * NdotL;
                }

                vec3 ambient = vec3(0.03) * material.albedo * material.ao;

                vec3 color = ambient + Lo;

                color = color / (color + vec3(1.0));
                color = pow(color, vec3(1.0 / 2.2));

                fragColor = vec4(color, 1.0);
            }
        )";

        auto shaderProgram = renderer.getDeviceManager().createShaderProgram(vs, fs);
        {
            auto pbrMaterial = renderer.getDeviceManager().createMaterial(shaderProgram);
            auto matres = resourceManager.createMaterial("pbrMaterial");

            // PBR test textures
            auto albedoImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/rustedmetal/albedo.png");
            albedoImage->load();
            auto normalImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/rustedmetal/normal.png");
            normalImage->load();
            auto metallicImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/rustedmetal/metallic.png");
            metallicImage->load();
            auto roughnessImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/rustedmetal/roughness.png");
            roughnessImage->load();
            auto aoImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/rustedmetal/ao.png");
            aoImage->load();

            // create textures from image data

            TextureDesc albedoTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, albedoImage->getWidth(), albedoImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto albedoTex = renderer.getDeviceManager().getDevice().createTexture(albedoTexDesc);
            albedoTex->upload(albedoImage->getData(), TextureRangeDesc::new2D(0, 0, albedoImage->getWidth(), albedoImage->getHeight()));

            TextureDesc normalTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, normalImage->getWidth(), normalImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto normalTex = renderer.getDeviceManager().getDevice().createTexture(normalTexDesc);
            normalTex->upload(normalImage->getData(), TextureRangeDesc::new2D(0, 0, normalImage->getWidth(), normalImage->getHeight()));

            TextureDesc metallicTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, metallicImage->getWidth(), metallicImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto metallicTex = renderer.getDeviceManager().getDevice().createTexture(metallicTexDesc);
            metallicTex->upload(metallicImage->getData(), TextureRangeDesc::new2D(0, 0, metallicImage->getWidth(), metallicImage->getHeight()));

            TextureDesc roughnessTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, roughnessImage->getWidth(), roughnessImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto roughnessTex = renderer.getDeviceManager().getDevice().createTexture(roughnessTexDesc);
            roughnessTex->upload(roughnessImage->getData(), TextureRangeDesc::new2D(0, 0, roughnessImage->getWidth(), roughnessImage->getHeight()));

            TextureDesc aoTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, aoImage->getWidth(), aoImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto aoTex = renderer.getDeviceManager().getDevice().createTexture(aoTexDesc);
            aoTex->upload(aoImage->getData(), TextureRangeDesc::new2D(0, 0, aoImage->getWidth(), aoImage->getHeight()));

            auto samplerState = renderer.getDevice().createSamplerState(SamplerStateDesc::newLinear());

            // set textures to material
            pbrMaterial->setTextureSampler("albedoMap", albedoTex, samplerState, 1);
            pbrMaterial->setTextureSampler("normalMap", normalTex, samplerState, 2);
            pbrMaterial->setTextureSampler("metallicMap", metallicTex, samplerState, 3);
            pbrMaterial->setTextureSampler("roughnessMap", roughnessTex, samplerState, 4);
            pbrMaterial->setTextureSampler("aoMap", aoTex, samplerState, 5);

            matres->setMaterial(pbrMaterial);
            matres->load();
        }

        {
            auto pbrMaterial = renderer.getDeviceManager().createMaterial(shaderProgram);
            auto matres = resourceManager.createMaterial("pbrMaterial2");

            // PBR test textures
            auto albedoImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/polishedconcrete/albedo.png");
            albedoImage->load();
            auto normalImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/polishedconcrete/normal.png");
            normalImage->load();
            auto metallicImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/default/metallic.png");
            metallicImage->load();
            auto roughnessImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/polishedconcrete/roughness.png");
            roughnessImage->load();
            auto aoImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/rustedmetal/ao.png");
            aoImage->load();

            // create textures from image data

            TextureDesc albedoTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, albedoImage->getWidth(), albedoImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto albedoTex = renderer.getDeviceManager().getDevice().createTexture(albedoTexDesc);
            albedoTex->upload(albedoImage->getData(), TextureRangeDesc::new2D(0, 0, albedoImage->getWidth(), albedoImage->getHeight()));

            TextureDesc normalTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, normalImage->getWidth(), normalImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto normalTex = renderer.getDeviceManager().getDevice().createTexture(normalTexDesc);
            normalTex->upload(normalImage->getData(), TextureRangeDesc::new2D(0, 0, normalImage->getWidth(), normalImage->getHeight()));

            TextureDesc metallicTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, metallicImage->getWidth(), metallicImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto metallicTex = renderer.getDeviceManager().getDevice().createTexture(metallicTexDesc);
            metallicTex->upload(metallicImage->getData(), TextureRangeDesc::new2D(0, 0, metallicImage->getWidth(), metallicImage->getHeight()));

            TextureDesc roughnessTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, roughnessImage->getWidth(), roughnessImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto roughnessTex = renderer.getDeviceManager().getDevice().createTexture(roughnessTexDesc);
            roughnessTex->upload(roughnessImage->getData(), TextureRangeDesc::new2D(0, 0, roughnessImage->getWidth(), roughnessImage->getHeight()));

            TextureDesc aoTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, aoImage->getWidth(), aoImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto aoTex = renderer.getDeviceManager().getDevice().createTexture(aoTexDesc);
            aoTex->upload(aoImage->getData(), TextureRangeDesc::new2D(0, 0, aoImage->getWidth(), aoImage->getHeight()));

            auto samplerState = renderer.getDevice().createSamplerState(SamplerStateDesc::newLinear());

            // set textures to material
            pbrMaterial->setTextureSampler("albedoMap", albedoTex, samplerState, 1);
            pbrMaterial->setTextureSampler("normalMap", normalTex, samplerState, 2);
            pbrMaterial->setTextureSampler("metallicMap", metallicTex, samplerState, 3);
            pbrMaterial->setTextureSampler("roughnessMap", roughnessTex, samplerState, 4);
            pbrMaterial->setTextureSampler("aoMap", aoTex, samplerState, 5);

            matres->setMaterial(pbrMaterial);
            matres->load();
        }
    }

    // normal material
    {
        // plain color material
        auto vs = R"(
            #version 450
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec3 inNormal;
            layout(location = 2) in vec2 inTexCoord;

            layout(location = 0) out vec3 fragNormal;

            layout(binding = 0) uniform UBO {
                mat4 model;
                mat4 view;
                mat4 proj;
            } ubo;

            void main() {
                gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
                mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
                fragNormal = normalMatrix * inNormal;
            }
        )";

        auto fs = R"(
            #version 450
            layout(location = 0) in vec3 fragNormal;
            out vec4 fragColor;

            void main() {
                fragColor = vec4(fragNormal * 0.5 + 0.5, 1.0);
            }
        )";

        auto shaderProgram = renderer.getDeviceManager().createShaderProgram(vs, fs);
        normalMaterial = renderer.getDeviceManager().createMaterial(shaderProgram);
        auto matres = resourceManager.createMaterial("normalMaterial");
        matres->setMaterial(normalMaterial);
        matres->load();
    }

    // test teapot material
    {
        auto vs = R"(
            #version 450
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec3 inNormal;
            layout(location = 2) in vec2 inTexCoord;

            layout(location = 0) out vec3 fragNormal;

            layout(binding = 0) uniform UBO {
                mat4 model;
                mat4 view;
                mat4 proj;
            } ubo;

            void main() {
                gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
                mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
                fragNormal = normalMatrix * inNormal;
            }
        )";

        auto fs = R"(
            #version 450
            layout(location = 0) in vec3 fragNormal;
            out vec4 fragColor;

            layout(binding = 1) uniform Constants {
                vec3 cameraPos;
                vec3 cameraDir;
                vec3 lightDir;
                float shininess;
            } constants;

            void main() {
                // shade the fragment based on the normal
                vec3 baseColor = vec3(0.6, 0.1, 0.1);
                vec3 shineColor = vec3(1.0);

                float intensity1 = pow(max(dot(normalize(fragNormal), normalize(constants.lightDir)) + 0.01, 0.025), constants.shininess);
                float intensity2 = min(pow(max(dot(normalize(fragNormal), normalize(constants.lightDir)), 0), 1.0), 1.0);
                float intensity = intensity1 + intensity2;
                vec3 finalColor = intensity1 * shineColor + intensity2 * baseColor;
                fragColor = vec4(finalColor, 1.0);
            }
        )";

        auto shaderProgram = renderer.getDeviceManager().createShaderProgram(vs, fs);
        testMaterial = renderer.getDeviceManager().createMaterial(shaderProgram);
        auto matres = resourceManager.createMaterial("testMaterial");
        matres->setMaterial(testMaterial);
        matres->load();
    }

    // floor material with checkered pattern
    {
        auto vs = R"(
            #version 450
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec3 inNormal;
            layout(location = 2) in vec2 inTexCoord;

            layout(location = 0) out vec3 fragNormal;
            layout(location = 1) out vec2 fragTexCoord;

            layout(binding = 0) uniform UBO {
                mat4 model;
                mat4 view;
                mat4 proj;
            } ubo;

            void main() {
                gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
                mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
                fragNormal = normalMatrix * inNormal;
                fragTexCoord = inTexCoord;
            }
        )";

        auto fs = R"(
            #version 450
            layout(location = 0) in vec3 fragNormal;
            layout(location = 1) in vec2 fragTexCoord;
            out vec4 fragColor;

            void main() {
                vec2 uv = fragTexCoord * 20.0;
                vec3 color = vec3(1.0);
                if (mod(int(uv.x) + int(uv.y), 2) == 0) {
                    color = vec3(0.0);
                }
                fragColor = vec4(color, 1.0);
            }
        )";

        auto shaderProgram = renderer.getDeviceManager().createShaderProgram(vs, fs);
        floorMaterial = renderer.getDeviceManager().createMaterial(shaderProgram);

        floorMaterial->setCullMode(CullMode::None);

        auto matres = resourceManager.createMaterial("floorMaterial");
        matres->setMaterial(floorMaterial);
        matres->load();
    }

    // portal material: usage will be a simple textured mesh
    {
        auto vs = R"(
            #version 450
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec3 inNormal;
            layout(location = 2) in vec2 inTexCoord;

            layout(location = 0) out vec3 fragNormal;
            layout(location = 1) out vec2 fragTexCoord;

            layout(binding = 0) uniform UBO {
                mat4 model;
                mat4 view;
                mat4 proj;
            } ubo;

            void main() {
                gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
                mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
                fragNormal = normalMatrix * inNormal;
                fragTexCoord = inTexCoord;
            }
        )";

        auto fs = R"(
            #version 450
            layout(location = 0) in vec3 fragNormal;
            layout(location = 1) in vec2 fragTexCoord;
            out vec4 fragColor;

            layout(binding = 1) uniform sampler2D tex;

            void main() {
                fragColor = texture(tex, fragTexCoord);
            }
        )";

        auto shaderProgram = renderer.getDeviceManager().createShaderProgram(vs, fs);
        portalMaterial = renderer.getDeviceManager().createMaterial(shaderProgram);
        auto matres = resourceManager.createMaterial("portalMaterial");
        matres->setMaterial(portalMaterial);
        matres->load();
    }

    {// Portal Mesh
        auto portalMesh = Mesh::createQuad();
        portalMesh->vertices[0].texCoords = {0.0f, 0.0f};
        portalMesh->vertices[1].texCoords = {1.0f, 0.0f};
        portalMesh->vertices[2].texCoords = {1.0f, 1.0f};
        portalMesh->vertices[3].texCoords = {0.0f, 1.0f};

        auto portalMeshRes = resourceManager.createMesh("portal");
        portalMeshRes->getMesh() = *portalMesh;

        graphics::VertexDataLayout attribLayout({
                { "inPosition", 0, VertexAttributeFormat::Float3 },
                { "inNormal", 1, VertexAttributeFormat::Float3 },
                { "inTexCoords", 2, VertexAttributeFormat::Float2 }
        });

        auto vertexData = renderer.getDeviceManager().createVertexData(attribLayout);
        vertexData->allocateVertexBuffer(renderer.getDevice(), portalMesh->vertices.size());
        vertexData->allocateIndexBuffer(renderer.getDevice(), portalMesh->indices.size());
        vertexData->pushVertices(portalMesh->vertices);
        vertexData->pushIndices(portalMesh->indices);

        portalMeshRes->setVertexData(vertexData);
        portalMeshRes->load();
    }


    // Load teapot model
    std::shared_ptr<Mesh> m = std::make_shared<Mesh>();
    {
        objl::Loader loader;
        bool success = loader.LoadFile(desc.assetPath + "/test/teapot.obj");
        if (!success)
        {
            std::cerr << "Failed to load model" << std::endl;
            return;
        }
        for (auto& LoadedVertice: loader.LoadedVertices)
        {
            Mesh::Vertex vertex{};
            vertex.position = {LoadedVertice.Position.X, LoadedVertice.Position.Y, LoadedVertice.Position.Z};
            vertex.normal = {LoadedVertice.Normal.X, LoadedVertice.Normal.Y, LoadedVertice.Normal.Z};
            vertex.texCoords = {LoadedVertice.TextureCoordinate.X, LoadedVertice.TextureCoordinate.Y};
            m->vertices.push_back(vertex);
        }
        for (unsigned int LoadedIndice: loader.LoadedIndices)
        {
            m->indices.push_back(LoadedIndice);
        }
        m->normalize();
        m->recalculateBounds();
    }
    {
        auto teapotMeshRes = resourceManager.createMesh("teapot");
        teapotMeshRes->getMesh().vertices = m->vertices;
        teapotMeshRes->getMesh().indices = m->indices;

        graphics::VertexDataLayout attribLayout({
                { "inPosition", 0, VertexAttributeFormat::Float3 },
                { "inNormal", 1, VertexAttributeFormat::Float3 },
                { "inTexCoords", 2, VertexAttributeFormat::Float2 }
        });

        auto vertexData = renderer.getDeviceManager().createVertexData(attribLayout);
        vertexData->allocateVertexBuffer(renderer.getDevice(), m->vertices.size());
        vertexData->allocateIndexBuffer(renderer.getDevice(), m->indices.size());
        vertexData->pushVertices(m->vertices);
        vertexData->pushIndices(m->indices);

        teapotMeshRes->setVertexData(vertexData);
        teapotMeshRes->load();
    }

    // Load spider mesh
    std::shared_ptr<Mesh> spiderMesh = std::make_shared<Mesh>();
    {
        objl::Loader loader;
        bool success = loader.LoadFile(desc.assetPath + "/test/cow.obj");
        if (!success)
        {
            std::cerr << "Failed to load model" << std::endl;
            return;
        }
        for (auto& LoadedVertice: loader.LoadedVertices)
        {
            Mesh::Vertex vertex{};
            vertex.position = {LoadedVertice.Position.X, LoadedVertice.Position.Y, LoadedVertice.Position.Z};
            vertex.normal = {LoadedVertice.Normal.X, LoadedVertice.Normal.Y, LoadedVertice.Normal.Z};
            vertex.texCoords = {LoadedVertice.TextureCoordinate.X, LoadedVertice.TextureCoordinate.Y};
            spiderMesh->vertices.push_back(vertex);
        }
        for (unsigned int LoadedIndice: loader.LoadedIndices)
        {
            spiderMesh->indices.push_back(LoadedIndice);
        }
        spiderMesh->normalize();
        spiderMesh->recalculateBounds();
    }
    {
        auto spiderMeshRes = resourceManager.createMesh("spider");
        spiderMeshRes->getMesh().vertices = spiderMesh->vertices;
        spiderMeshRes->getMesh().indices = spiderMesh->indices;

        graphics::VertexDataLayout attribLayout({
                { "inPosition", 0, VertexAttributeFormat::Float3 },
                { "inNormal", 1, VertexAttributeFormat::Float3 },
                { "inTexCoords", 2, VertexAttributeFormat::Float2 }
        });

        auto vertexData = renderer.getDeviceManager().createVertexData(attribLayout);
        vertexData->allocateVertexBuffer(renderer.getDevice(), spiderMesh->vertices.size());
        vertexData->allocateIndexBuffer(renderer.getDevice(), spiderMesh->indices.size());
        vertexData->pushVertices(spiderMesh->vertices);
        vertexData->pushIndices(spiderMesh->indices);

        spiderMeshRes->setVertexData(vertexData);
        spiderMeshRes->load();
    }

    // standford bunny mesh
    std::shared_ptr<Mesh> bunnyMesh = std::make_shared<Mesh>();
    {
        objl::Loader loader;
        bool success = loader.LoadFile(desc.assetPath + "/test/suzanne.obj");
        if (!success)
        {
            std::cerr << "Failed to load model" << std::endl;
            //return;
        }
        for (auto& LoadedVertice: loader.LoadedVertices)
        {
            Mesh::Vertex vertex{};
            vertex.position = {LoadedVertice.Position.X, LoadedVertice.Position.Y, LoadedVertice.Position.Z};
            vertex.normal = {LoadedVertice.Normal.X, LoadedVertice.Normal.Y, LoadedVertice.Normal.Z};
            vertex.texCoords = {LoadedVertice.TextureCoordinate.X, LoadedVertice.TextureCoordinate.Y};
            bunnyMesh->vertices.push_back(vertex);
        }
        for (unsigned int LoadedIndice: loader.LoadedIndices)
        {
            bunnyMesh->indices.push_back(LoadedIndice);
        }
        bunnyMesh->normalize();
        bunnyMesh->recalculateBounds();
    }
    {
        auto bunnyMeshRes = resourceManager.createMesh("bunny");
        bunnyMeshRes->getMesh().vertices = bunnyMesh->vertices;
        bunnyMeshRes->getMesh().indices = bunnyMesh->indices;

        graphics::VertexDataLayout attribLayout({
                { "inPosition", 0, VertexAttributeFormat::Float3 },
                { "inNormal", 1, VertexAttributeFormat::Float3 },
                { "inTexCoords", 2, VertexAttributeFormat::Float2 }
        });

        auto vertexData = renderer.getDeviceManager().createVertexData(attribLayout);
        vertexData->allocateVertexBuffer(renderer.getDevice(), bunnyMesh->vertices.size());
        vertexData->allocateIndexBuffer(renderer.getDevice(), bunnyMesh->indices.size());
        vertexData->pushVertices(bunnyMesh->vertices);
        vertexData->pushIndices(bunnyMesh->indices);

        bunnyMeshRes->setVertexData(vertexData);
        bunnyMeshRes->load();
    }

    {
        auto sphereMeshRes = resourceManager.createMesh("sphere");
        sphereMeshRes->getMesh() = *Mesh::createSphere(2.0f);

        graphics::VertexDataLayout attribLayout({
                { "inPosition", 0, VertexAttributeFormat::Float3 },
                { "inNormal", 1, VertexAttributeFormat::Float3 },
                { "inTexCoords", 2, VertexAttributeFormat::Float2 }
        });

        auto vertexData = renderer.getDeviceManager().createVertexData(attribLayout);
        vertexData->allocateVertexBuffer(renderer.getDevice(), sphereMeshRes->getMesh().vertices.size());
        vertexData->allocateIndexBuffer(renderer.getDevice(), sphereMeshRes->getMesh().indices.size());
        vertexData->pushVertices(sphereMeshRes->getMesh().vertices);
        vertexData->pushIndices(sphereMeshRes->getMesh().indices);

        sphereMeshRes->setVertexData(vertexData);
        sphereMeshRes->load();
    }

    {
        auto floorMeshRes = resourceManager.createMesh("floor");
        floorMeshRes->getMesh() = *Mesh::createQuad(10.0f);

        graphics::VertexDataLayout attribLayout({
                { "inPosition", 0, VertexAttributeFormat::Float3 },
                { "inNormal", 1, VertexAttributeFormat::Float3 },
                { "inTexCoords", 2, VertexAttributeFormat::Float2 }
        });

        auto vertexData = renderer.getDeviceManager().createVertexData(attribLayout);
        vertexData->allocateVertexBuffer(renderer.getDevice(), floorMeshRes->getMesh().vertices.size());
        vertexData->allocateIndexBuffer(renderer.getDevice(), floorMeshRes->getMesh().indices.size());
        vertexData->pushVertices(floorMeshRes->getMesh().vertices);
        vertexData->pushIndices(floorMeshRes->getMesh().indices);

        floorMeshRes->setVertexData(vertexData);
        floorMeshRes->load();
    }

    {
        auto portalFrameMeshRes = resourceManager.createMesh("portalFrame");
        portalFrameMeshRes->getMesh() = *Mesh::createCube(1.0f);

        graphics::VertexDataLayout attribLayout({
                { "inPosition", 0, VertexAttributeFormat::Float3 },
                { "inNormal", 1, VertexAttributeFormat::Float3 },
                { "inTexCoords", 2, VertexAttributeFormat::Float2 }
        });

        auto vertexData = renderer.getDeviceManager().createVertexData(attribLayout);
        vertexData->allocateVertexBuffer(renderer.getDevice(), portalFrameMeshRes->getMesh().vertices.size());
        vertexData->allocateIndexBuffer(renderer.getDevice(), portalFrameMeshRes->getMesh().indices.size());
        vertexData->pushVertices(portalFrameMeshRes->getMesh().vertices);
        vertexData->pushIndices(portalFrameMeshRes->getMesh().indices);

        portalFrameMeshRes->setVertexData(vertexData);
        portalFrameMeshRes->load();
    }

    //create a number of meshes in the resource manager
    {
        int numMeshes = 10000;
        for (int i = 0; i < numMeshes; i++)
        {
            auto mesh = Mesh::createCube(1.0f);
            auto meshRes = resourceManager.createMesh("cube" + std::to_string(i));
            meshRes->getMesh() = *mesh;

            graphics::VertexDataLayout attribLayout({
                    { "inPosition", 0, VertexAttributeFormat::Float3 },
                    { "inNormal", 1, VertexAttributeFormat::Float3 },
                    { "inTexCoords", 2, VertexAttributeFormat::Float2 }
            });

            auto vertexData = renderer.getDeviceManager().createVertexData(attribLayout);
            vertexData->allocateVertexBuffer(renderer.getDevice(), mesh->vertices.size());
            vertexData->allocateIndexBuffer(renderer.getDevice(), mesh->indices.size());
            vertexData->pushVertices(mesh->vertices);
            vertexData->pushIndices(mesh->indices);

            meshRes->setVertexData(vertexData);
            meshRes->load();
        }
    }

    // Create a scene
    defaultScene = std::make_shared<Scene>();


    // spawn a bunch of teapots at random positions and rotations
    {
        Light light;
        int num = 0;
        for (int i = 0; i < num; i++)
        {
            EntityView teapot = defaultScene->createEntity("teapot" + std::to_string(i));
            teapot.addComponent<MeshComponent>(resourceManager.getMeshByName("teapot"), resourceManager.getMaterialByName("testMaterial"));
            teapot.addComponent<LightComponent>(light);
            auto& teapotNode = teapot.getSceneNode();
            teapotNode.getTransform().setPosition({(float)rand() / RAND_MAX * 20.0f - 10.0f, (float)rand() / RAND_MAX * 20.0f - 10.0f, (float)rand() / RAND_MAX * 20.0f - 10.0f});
            teapotNode.getTransform().setRotation({(float)rand() / RAND_MAX * 360.0f, (float)rand() / RAND_MAX * 360.0f, (float)rand() / RAND_MAX * 360.0f});
        }
    }

    // teapot gobbledygook
    {
        EntityView viewer = defaultScene->createEntity("viewer");
        viewer.addComponent<CameraComponent>(activeCamera);
        viewer.addComponent<LightComponent>(Light{});
        viewer.getSceneNode().getTransform().setPosition({0.0f, 6.0f, 3.0f});
        viewer.getSceneNode().getTransform().setRotation({glm::radians(-20.0f), 0, 0.0f});

        EntityView cow = defaultScene->createEntity("cow");
        cow.addComponent<MeshComponent>(resourceManager.getMeshByName("spider"), resourceManager.getMaterialByName("testMaterial"));
        cow.getSceneNode().getTransform().setPosition({-7.0f, 0.0f, -4.0f});
        cow.getSceneNode().getTransform().setScale(glm::vec3(1.0f));
        cow.getSceneNode().getTransform().setRotation({0.0f, glm::radians(20.0f), 0.0f});

        EntityView bunny = defaultScene->createEntity("bunny");
        bunny.addComponent<MeshComponent>(resourceManager.getMeshByName("bunny"), resourceManager.getMaterialByName("testMaterial"));
        bunny.getSceneNode().getTransform().setPosition({0.0f, 0.0f, 0.0f});
        bunny.getSceneNode().getTransform().setScale(glm::vec3(2.0f));
        bunny.getSceneNode().getTransform().setRotation({0.0f, 3.0f, 0.0f});

        EntityView root = defaultScene->createEntity("teapot");
        root.addComponent<MeshComponent>(resourceManager.getMeshByName("teapot"), resourceManager.getMaterialByName("testMaterial"));

        auto& rootNode = root.getSceneNode();
        rootNode.getTransform().setPosition({-3.0f, 0.0f, 10.0f});
        rootNode.getTransform().setScale({1.f, 1.f, 1.f});
        rootNode.getTransform().setRotation({0.0f, 0.0f, 0.0f});

        // Create a child node
        EntityView child = defaultScene->createEntity("childTeapot");

        child.addComponent<MeshComponent>(resourceManager.getMeshByName("teapot"), resourceManager.getMaterialByName("testMaterial"));
        auto& childNode = child.getSceneNode();
        childNode.getTransform().setPosition({7.0f, 0.0f, 0.0f});
        childNode.getTransform().setScale({1.f, 1.f, 1.f});
        childNode.getTransform().setRotation({0.0f, 0.0f, 0.0f});

        EntityView teapotPOV = defaultScene->createEntity("teapotPOV");
        auto& teapotPOVNode = teapotPOV.getSceneNode();
        {
            auto testRenderTextureCamera = std::make_shared<Camera>("testRenderTextureCamera");
            testRenderTexture = renderer.getDevice().createTexture(TextureDesc::new2D(TextureFormat::RGBA_UNorm8, desc.width, desc.height, TextureDesc::TextureUsageBits::Attachment | TextureDesc::TextureUsageBits::Sampled));
            auto testRenderTextureCameraTarget = graphics::RenderTarget{
                    .colorTexture = testRenderTexture,
                    .clearColor = {0.0f, 0.2f, 0.2f, 1.0f},
            };
            teapotPOV.addComponent<CameraComponent>(testRenderTextureCamera, testRenderTextureCameraTarget);
            teapotPOV.addComponent<MeshComponent>(resourceManager.getMeshByName("teapot"), resourceManager.getMaterialByName("testMaterial"));

            auto& teapotPOVTransform = teapotPOVNode.getTransform();
            teapotPOVTransform.setPosition({4.0f, 0.0f, 0.0f});
            teapotPOVTransform.setRotation({0.0f, glm::radians(90.0f), 0.0f});
        }
        childNode.addChild(&teapotPOVNode);


        // Create a child node
        EntityView spherePortal = defaultScene->createEntity("spherePortal");

        spherePortal.addComponent<MeshComponent>(resourceManager.getMeshByName("sphere"), resourceManager.getMaterialByName("pbrMaterial"));
        auto& spherePortalNode = spherePortal.getSceneNode();
        spherePortalNode.getTransform().setPosition({10.0f, 3.0f, 10.0f});
        spherePortalNode.getTransform().setScale({1.f, 1.f, 1.f});
        spherePortalNode.getTransform().setRotation({0.0f, 0.0f, 0.0f});

        // Create a child node
        EntityView sphere = defaultScene->createEntity("sphere");

        sphere.addComponent<MeshComponent>(resourceManager.getMeshByName("sphere"), resourceManager.getMaterialByName("testMaterial"));
        Light light;
        light.setColor({1.0f, 0.0f, 1.0f});
        sphere.addComponent<LightComponent>(light);
        auto& sphereNode = sphere.getSceneNode();
        sphereNode.getTransform().setPosition({10.0f, 0.0f, 0.0f});
        sphereNode.getTransform().setScale({1.f, 1.f, 1.f});
        sphereNode.getTransform().setRotation({0.0f, 0.0f, 0.0f});

        childNode.addChild(&sphereNode);


        rootNode.addChild(&childNode);

        viewer.getSceneNode().getTransform().lookAt(rootNode.getTransform().getPosition(), {0.0f, 1.0f, 0.0f});
    }

    // floor
    {
        auto floor = defaultScene->createEntity("floor");
        floor.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("pbrMaterial2"));
        {
            auto& floorNode = floor.getSceneNode();
            floorNode.getTransform().setPosition({0.0f, -5.0f, 0.0f});
            floorNode.getTransform().setScale({20.0f, 20.0f, 1.0f});
            floorNode.getTransform().setRotation({glm::radians(90.0f), 0.0f, 0.0f});
        }
    }

    // portal and its frame
    {
        auto portal = defaultScene->createEntity("portal");
        portal.addComponent<MeshComponent>(resourceManager.getMeshByName("portal"), resourceManager.getMaterialByName("portalMaterial"));
        {
            auto& portalNode = portal.getSceneNode();
            portalNode.getTransform().setPosition({0.0f, 0.0f, 0.0f});
            portalNode.getTransform().setScale({5.0f, 5.0f, 1.0f});
            portalNode.getTransform().setRotation({glm::radians(90.0f), 0.0f, 0.0f});

            auto& portalMaterialComponent = portal.getComponent<MeshComponent>();
            auto portalMaterial_ = portalMaterialComponent.getMaterial()->getMaterial();
            auto samplerState = renderer.getDevice().createSamplerState(SamplerStateDesc::newLinear());
            portalMaterial_->setTextureSampler("tex", testRenderTexture, samplerState, 0);
        }

        // create a rectangular frame around the portal made of 4 cubes stretched to be a frame
        auto portalFrame = defaultScene->createEntity("portalFrame");

        // transform the frame pieces according to i
        for (int i = 0; i < 4; i++)
        {
            auto framePart = defaultScene->createEntity("framePart" + std::to_string(i));
            framePart.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("normalMaterial"));
            auto& framePartNode = framePart.getSceneNode();

            // transform the frame part according to i, add a scale parameter for the thickness, and the transformations are TRS
            switch (i)
            {
                case 0:
                    framePartNode.getTransform().setPosition({0.0f, 0.0f, -5.0f});
                    framePartNode.getTransform().setScale({5.0f, 0.1f, 0.1f});
                    break;
                case 1:
                    framePartNode.getTransform().setPosition({0.0f, 0.0f, 5.0f});
                    framePartNode.getTransform().setScale({5.0f, 0.1f, 0.1f});
                    break;
                case 2:
                    framePartNode.getTransform().setPosition({-5.0f, 0.0f, 0.0f});
                    framePartNode.getTransform().setScale({0.1f, 0.1f, 5.0f});
                    break;
                case 3:
                    framePartNode.getTransform().setPosition({5.0f, 0.0f, 0.0f});
                    framePartNode.getTransform().setScale({0.1f, 0.1f, 5.0f});
                    break;
            }

            portalFrame.getSceneNode().addChild(&framePartNode);
            portalFrame.getSceneNode().getTransform().setRotation({glm::radians(90.0f), 0.0f, 0.0f});
        }
        portalFrame.getSceneNode().addChild(&portal.getSceneNode());
        portalFrame.getSceneNode().getTransform().setPosition({20.0f, 0.0f, 0.0f});
//        portal.getSceneNode().addChild(&portalFrame.getSceneNode());
    }

    stage.setScene(defaultScene);
}

void EngineInstance::updateSimulation(float dt)
{
//    std::cout << "Updating simulation (" << (dt * 1000.0f) << " ms)" << std::endl;

    // We can move things around in our testing scene
    {
        std::optional<EntityView> root_ = defaultScene->getEntityByName("teapot");
        if (root_)
        {
            auto& root = root_->getSceneNode();
            root.getTransform().rotate(glm::angleAxis(glm::radians(1.0f), glm::vec3(0.0f, 0.0f, 1.0f)));

            auto* childNode_ = root.findNode("childTeapot");
            if (childNode_)
            {
                auto& childNode = *childNode_;
                childNode.getTransform().rotate(glm::angleAxis(glm::radians(4.0f), glm::vec3(0.0f, 1.0f, -1.0f)));
            }
        }
    }

    // slowly rotating portal
    {
        std::optional<EntityView> portalFrame_ = defaultScene->getEntityByName("portalFrame");
        if (portalFrame_)
        {
            auto& portalFrameNode = portalFrame_->getSceneNode();
            portalFrameNode.getTransform().rotate(glm::angleAxis(glm::radians(0.5f), glm::vec3(0.0f, 1.0f, 0.0f)));
        }

//        auto portal = defaultScene->getEntityByName("portal");
//        portal->getSceneNode().getTransform().rotate(glm::angleAxis(glm::radians(0.5f), glm::vec3(1.0f, 0.0f, 0.0f)));
    }

    auto viewer = defaultScene->getEntityByName("viewer");
    if (viewer)
    {
        auto& viewerNode = viewer->getSceneNode();
        // make it turn in circles with system clock
//        viewerNode.getTransform().setPosition({10.0f * glm::cos((float)clock()/1000.0f), 6.0f, 10.0f * glm::sin((float)clock()/1000.0f)});
//        viewerNode.getTransform().lookAt({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});


        if (input.isMouseDragging(0))
        {
            std::cout << "Mouse dragging" << std::endl;
            std::cout << "Mouse drag delta: " << input.getMouseDragDeltaX() << ", " << input.getMouseDragDeltaY() << std::endl;
            auto& transform = viewerNode.getTransform();

            // Create quaternions representing the x and y rotations
//            glm::quat xRotation = glm::angleAxis(input.getMouseDragDeltaY() * 0.01f, glm::vec3(1.0f, 0.0f, 0.0f));  // X rotation around the right axis
//            glm::quat yRotation = glm::angleAxis(input.getMouseDragDeltaX() * 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));  // Y rotation around the up axis

            pitch = input.getMouseDragDeltaY() * 0.01f;
            yaw = input.getMouseDragDeltaX() * 0.01f;

            glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1, 0, 0));
            glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0, 1, 0));
            glm::quat qRoll = glm::angleAxis(roll,glm::vec3(0,0,1));

            //For a FPS camera we can omit roll
//            glm::quat orientation = qRoll * qPitch * qYaw;

//            transform.setRotation(glm::normalize(orientation));

            transform.setRotation(glm::normalize(qYaw * transform.getRotation()));
            transform.setRotation(glm::normalize(transform.getRotation() * qPitch));
        }

        // wasd
        float speed = 0.1f;
        if (input.isKeyPressed(input::KeyCode::W))
        {
            // move forward relative to orientation
            viewerNode.getTransform().translate(viewerNode.getTransform().getForward() * -speed);
        }
        if (input.isKeyPressed(input::KeyCode::S))
        {
            // move backward relative to orientation
            viewerNode.getTransform().translate(viewerNode.getTransform().getForward() * speed);
        }
        if (input.isKeyPressed(input::KeyCode::A))
        {
            // move left relative to orientation
            viewerNode.getTransform().translate(viewerNode.getTransform().getRight() * -speed);
        }
        if (input.isKeyPressed(input::KeyCode::D))
        {
            // move right relative to orientation
            viewerNode.getTransform().translate(viewerNode.getTransform().getRight() * speed);
        }

        // up and down
        if (input.isKeyPressed(input::KeyCode::Q))
        {
            // move up
            viewerNode.getTransform().translate(viewerNode.getTransform().getUp() * speed);
        }
        if (input.isKeyPressed(input::KeyCode::E))
        {
            // move down
            viewerNode.getTransform().translate(viewerNode.getTransform().getUp() * -speed);
        }
    }

    stage.update(dt);
}

void EngineInstance::renderFrame()
{
    if (!renderer.isInitialized())
    {
        // log error
        return;
    }


    if (auto* activeScene = stage.getScene())
    {
//        SceneRenderData sceneRenderData;
//        activeScene->getSceneRenderData(sceneRenderData);
//
//        sceneRenderer.render(renderer, sceneRenderData);
        // Render the scene for all cameras
        auto cameras = activeScene->getCameraNodes();

        SceneNode* mainCamera = nullptr;
        for (auto& cameraNode: cameras)
        {
            auto& camera = cameraNode->getEntityView().getComponent<CameraComponent>();
            if (camera.getRenderTarget().colorTexture == nullptr)
            {
                mainCamera = cameraNode;
                continue;
            }

            SceneRenderData sceneRenderData;
            activeScene->getSceneRenderData(sceneRenderData);

            // update pbr lights
            {
                auto pbrMaterial = resourceManager.getMaterialByName("pbrMaterial")->getMaterial();

                constexpr int MAX_LIGHTS = 10;

                struct LightStruct
                {
                    alignas(16) glm::vec3 position;
                    alignas(16) glm::vec3 color;
                };
                struct LightsUBO
                {
                    int numLights;
                    LightStruct lights[MAX_LIGHTS];
                };

                LightsUBO lightsUBO{};

                for (int i = 0; i < sceneRenderData.lights.size(); i++)
                {
                    auto item = sceneRenderData.lights[i];
                    LightStruct light{};
                    light.position = item.position;
                    light.color = item.light->getColor();
                    lightsUBO.lights[i] = light;
                    ++lightsUBO.numLights;
                }

                pbrMaterial->setUniformBytes("Lights", &lightsUBO, sizeof(lightsUBO), 6); // binding 6
            }

            renderer.begin(camera.getRenderTarget());
            sceneRenderer.render(renderer, sceneRenderData, {cameraNode->getWorldTransform().getPosition(),
                                                             cameraNode->getWorldTransform().getForward(),
                                                             glm::inverse(cameraNode->getWorldTransform().getModel()), camera.getCamera()->getProjection(), camera.getCamera()->getViewportWidth(), camera.getCamera()->getViewportHeight()});
            renderer.end();
        }

        if (mainCamera)
        {
            auto& camera = mainCamera->getEntityView().getComponent<CameraComponent>();
            SceneRenderData sceneRenderData;
            activeScene->getSceneRenderData(sceneRenderData);
            renderer.begin();
            renderer.bindViewport({0,0, static_cast<float>(desc.width), static_cast<float>(desc.height)});
            sceneRenderer.render(renderer, sceneRenderData, {mainCamera->getWorldTransform().getPosition(),
                                                             mainCamera->getWorldTransform().getForward(),
                                                             glm::inverse(mainCamera->getWorldTransform().getModel()), camera.getCamera()->getProjection(), camera.getCamera()->getViewportWidth(), camera.getCamera()->getViewportHeight()});
            renderer.end();
        }
    }
    else
    {
        // log error
    }
}

void EngineInstance::shutdown()
{
    renderer.shutdown();
}

graphics::Renderer& EngineInstance::getRenderer()
{
    return renderer;
}

Stage& EngineInstance::getStage()
{
    return stage;
}

Input& EngineInstance::getInput()
{
    return input;
}
