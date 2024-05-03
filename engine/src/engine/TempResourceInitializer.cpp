//
// Created by Jonathan Richard on 2024-05-03.
//

#include "TempResourceInitializer.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "util/tiny_obj_loader.h"


namespace TempResourceInitializer
{

void loadShaderFile(const std::string& file, std::string& out)
{
    std::ifstream shaderFile(file);
    if (!shaderFile.is_open())
    {
        throw std::runtime_error("Failed to open shader file: " + file);
    }

    std::string line;
    while (std::getline(shaderFile, line))
    {
        out += line + "\n";
    }
}

void loadObj(std::string file, Mesh& mesh)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Mesh::Vertex vertex{};
            vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };

            if (!attrib.normals.empty())
            {
                vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                };
            }

            if (!attrib.texcoords.empty())
            {
                vertex.texCoords = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            mesh.vertices.push_back(vertex);
            mesh.indices.push_back(mesh.indices.size());
        }
    }
    if (attrib.normals.empty())
    {
        mesh.normalize();
    }

}
void init(ResourceManager& resourceManager, graphics::Renderer& renderer, const InstanceDesc& desc)
{
    // Create materials

    // create default textures
    {
        {
            auto tex = renderer.getDeviceManager().getDevice().createTexture(TextureDesc::new2D(TextureFormat::RGBA_UNorm8, 1, 1, TextureDesc::TextureUsageBits::Sampled));
            uint8_t data[] = {255, 255, 255, 0};
            tex->upload(data, TextureRangeDesc::new2D(0, 0, 1, 1));
            auto texRes = resourceManager.createTexture("emtpyTexture");
            texRes->loadFromManagedResource(tex, renderer.getDeviceManager().getDevice().createSamplerState(SamplerStateDesc::newLinear()));
        }


        {
            auto tex = renderer.getDeviceManager().getDevice().createTexture(TextureDesc::new2D(TextureFormat::RGBA_UNorm8, 1, 1, TextureDesc::TextureUsageBits::Sampled));
            uint8_t data[] = {255, 0, 255, 255};
            tex->upload(data, TextureRangeDesc::new2D(0, 0, 1, 1));
            auto texRes = resourceManager.createTexture("missingTexture");
            texRes->loadFromManagedResource(tex, renderer.getDeviceManager().getDevice().createSamplerState(SamplerStateDesc::newLinear()));
        }
    }

    // PBR material
    {
        std::string vs;
        loadShaderFile(desc.assetPath + "/test/shaders/lighting.vert", vs);

        std::string fs;
        loadShaderFile(desc.assetPath + "/test/shaders/lighting.frag", fs);

        auto shaderRes = resourceManager.createShader("pbrShader");
        shaderRes->loadFromManagedResource(renderer.getDeviceManager().createShaderProgram(vs, fs));

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

        {
            auto matres = resourceManager.createMaterial("pbrDefaultMaterial");
            matres->loadFromManagedResource(renderer.getDeviceManager().createMaterial(nullptr));
            matres->setShader(shaderRes);

            PBRSettings settings;
            settings.useAlbedoMap = false;
            settings.useNormalMap = false;
            settings.useMetallicMap = false;
            settings.useRoughnessMap = false;
            settings.useAOMap = false;
            settings.useEmissiveMap = false;

            settings.baseColor = glm::vec3(0.5f, 0.0f, 0.0f);
            settings.emissionIntensity = 0.0f;
            matres->setUniformBuffer("Settings", &settings, sizeof(PBRSettings), 2);
        }

        {
            auto matres = resourceManager.createMaterial("pbrBlack");
            matres->loadFromManagedResource(renderer.getDeviceManager().createMaterial(nullptr));
            matres->setShader(shaderRes);

            PBRSettings settings;
            settings.useAlbedoMap = false;
            settings.useNormalMap = false;
            settings.useMetallicMap = false;
            settings.useRoughnessMap = false;
            settings.useAOMap = false;
            settings.useEmissiveMap = false;

            settings.metallic = 0.875f;
            settings.roughness = 0.01f;
            settings.ao = 1.0f;
            settings.baseColor = glm::vec3(0.0f, 0.0f, 0.0f);
            settings.emissionColor = glm::vec3(0.f);
            settings.emissionIntensity = 0.0f;
            matres->setUniformBuffer("Settings", &settings, sizeof(PBRSettings), 2);
        }


        {
            auto matres = resourceManager.createMaterial("pbrGlowMaterial");
            matres->loadFromManagedResource(renderer.getDeviceManager().createMaterial(nullptr));
            matres->setShader(shaderRes);

            PBRSettings settings;
            settings.useAlbedoMap = false;
            settings.useNormalMap = false;
            settings.useMetallicMap = false;
            settings.useRoughnessMap = false;
            settings.useAOMap = false;
            settings.useEmissiveMap = false;

            settings.baseColor = glm::vec3(1.0f, 0.0f, 0.0f);
            settings.emissionColor = settings.baseColor;
            settings.emissionIntensity = 10.0f;
            matres->setUniformBuffer("Settings", &settings, sizeof(PBRSettings), 2);
        }

        {
            auto matres = resourceManager.createMaterial("pbrGlowMaterialBlue");
            matres->loadFromManagedResource(renderer.getDeviceManager().createMaterial(nullptr));
            matres->setShader(shaderRes);

            PBRSettings settings;
            settings.useAlbedoMap = false;
            settings.useNormalMap = false;
            settings.useMetallicMap = false;
            settings.useRoughnessMap = false;
            settings.useAOMap = false;
            settings.useEmissiveMap = false;

            settings.baseColor = glm::vec3(0.01f, 0.01f, 1.0f);
            settings.emissionColor = settings.baseColor;
            settings.emissionIntensity = 20.0f;
            matres->setUniformBuffer("Settings", &settings, sizeof(PBRSettings), 2);
        }


        {
            auto matres = resourceManager.createMaterial("pbrGlowMaterialGreen");
            matres->loadFromManagedResource(renderer.getDeviceManager().createMaterial(nullptr));
            matres->setShader(shaderRes);

            PBRSettings settings;
            settings.useAlbedoMap = false;
            settings.useNormalMap = false;
            settings.useMetallicMap = false;
            settings.useRoughnessMap = false;
            settings.useAOMap = false;
            settings.useEmissiveMap = false;

            settings.baseColor = glm::vec3(0.0f, 1.0f, 0.0f);
            settings.emissionColor = settings.baseColor;
            settings.emissionIntensity = 5.0f;
            matres->setUniformBuffer("Settings", &settings, sizeof(PBRSettings), 2);
        }

        {
            auto matres = resourceManager.createMaterial("pbrMaterial");
            matres->loadFromManagedResource(renderer.getDeviceManager().createMaterial(nullptr));

            matres->setShader(shaderRes);

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
            auto samplerState = renderer.getDevice().createSamplerState(SamplerStateDesc::newLinear());

            TextureDesc albedoTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, albedoImage->getWidth(), albedoImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto albedoTex = renderer.getDeviceManager().getDevice().createTexture(albedoTexDesc);
            albedoTex->upload(albedoImage->getData(), TextureRangeDesc::new2D(0, 0, albedoImage->getWidth(), albedoImage->getHeight()));
            auto albedoTexRes = resourceManager.createTexture("rustedmetal/albedoMap");
            albedoTexRes->loadFromManagedResource(albedoTex, samplerState);

            TextureDesc normalTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, normalImage->getWidth(), normalImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto normalTex = renderer.getDeviceManager().getDevice().createTexture(normalTexDesc);
            normalTex->upload(normalImage->getData(), TextureRangeDesc::new2D(0, 0, normalImage->getWidth(), normalImage->getHeight()));
            auto normalTexRes = resourceManager.createTexture("rustedmetal/normalMap");
            normalTexRes->loadFromManagedResource(normalTex, samplerState);

            TextureDesc metallicTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, metallicImage->getWidth(), metallicImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto metallicTex = renderer.getDeviceManager().getDevice().createTexture(metallicTexDesc);
            metallicTex->upload(metallicImage->getData(), TextureRangeDesc::new2D(0, 0, metallicImage->getWidth(), metallicImage->getHeight()));
            auto metallicTexRes = resourceManager.createTexture("rustedmetal/metallicMap");
            metallicTexRes->loadFromManagedResource(metallicTex, samplerState);

            TextureDesc roughnessTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, roughnessImage->getWidth(), roughnessImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto roughnessTex = renderer.getDeviceManager().getDevice().createTexture(roughnessTexDesc);
            roughnessTex->upload(roughnessImage->getData(), TextureRangeDesc::new2D(0, 0, roughnessImage->getWidth(), roughnessImage->getHeight()));
            auto roughnessTexRes = resourceManager.createTexture("rustedmetal/roughnessMap");
            roughnessTexRes->loadFromManagedResource(roughnessTex, samplerState);

            TextureDesc aoTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, aoImage->getWidth(), aoImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto aoTex = renderer.getDeviceManager().getDevice().createTexture(aoTexDesc);
            aoTex->upload(aoImage->getData(), TextureRangeDesc::new2D(0, 0, aoImage->getWidth(), aoImage->getHeight()));
            auto aoTexRes = resourceManager.createTexture("rustedmetal/aoMap");
            aoTexRes->loadFromManagedResource(aoTex, samplerState);

            // set textures to material
            matres->setTextureSampler("albedoMap", albedoTexRes, 1);
            matres->setTextureSampler("normalMap", normalTexRes, 2);
            matres->setTextureSampler("metallicMap", metallicTexRes, 3);
            matres->setTextureSampler("roughnessMap", roughnessTexRes, 4);
            matres->setTextureSampler("aoMap", aoTexRes, 5);

            PBRSettings settings;
            settings.useEmissiveMap = false;
            matres->setUniformBuffer("Settings", &settings, sizeof(PBRSettings), 2);
        }

        {
            auto matres = resourceManager.createMaterial("pbrMaterial2");
            matres->loadFromManagedResource(renderer.getDeviceManager().createMaterial(nullptr));

            matres->setShader(shaderRes);

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
            auto samplerState = renderer.getDevice().createSamplerState(SamplerStateDesc::newLinear());

            TextureDesc albedoTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, albedoImage->getWidth(), albedoImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto albedoTex = renderer.getDeviceManager().getDevice().createTexture(albedoTexDesc);
            albedoTex->upload(albedoImage->getData(), TextureRangeDesc::new2D(0, 0, albedoImage->getWidth(), albedoImage->getHeight()));
            auto albedoTexRes = resourceManager.createTexture("polishedconcrete/albedoMap");
            albedoTexRes->loadFromManagedResource(albedoTex, samplerState);

            TextureDesc normalTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, normalImage->getWidth(), normalImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto normalTex = renderer.getDeviceManager().getDevice().createTexture(normalTexDesc);
            normalTex->upload(normalImage->getData(), TextureRangeDesc::new2D(0, 0, normalImage->getWidth(), normalImage->getHeight()));
            auto normalTexRes = resourceManager.createTexture("polishedconcrete/normalMap");
            normalTexRes->loadFromManagedResource(normalTex, samplerState);

            TextureDesc metallicTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, metallicImage->getWidth(), metallicImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto metallicTex = renderer.getDeviceManager().getDevice().createTexture(metallicTexDesc);
            metallicTex->upload(metallicImage->getData(), TextureRangeDesc::new2D(0, 0, metallicImage->getWidth(), metallicImage->getHeight()));
            auto metallicTexRes = resourceManager.createTexture("polishedconcrete/metallicMap");
            metallicTexRes->loadFromManagedResource(metallicTex, samplerState);

            TextureDesc roughnessTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, roughnessImage->getWidth(), roughnessImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto roughnessTex = renderer.getDeviceManager().getDevice().createTexture(roughnessTexDesc);
            roughnessTex->upload(roughnessImage->getData(), TextureRangeDesc::new2D(0, 0, roughnessImage->getWidth(), roughnessImage->getHeight()));
            auto roughnessTexRes = resourceManager.createTexture("polishedconcrete/roughnessMap");
            roughnessTexRes->loadFromManagedResource(roughnessTex, samplerState);

            TextureDesc aoTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, aoImage->getWidth(), aoImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto aoTex = renderer.getDeviceManager().getDevice().createTexture(aoTexDesc);
            aoTex->upload(aoImage->getData(), TextureRangeDesc::new2D(0, 0, aoImage->getWidth(), aoImage->getHeight()));
            auto aoTexRes = resourceManager.createTexture("polishedconcrete/aoMap");
            aoTexRes->loadFromManagedResource(aoTex, samplerState);


            // set textures to material
            matres->setTextureSampler("albedoMap", albedoTexRes, 1);
            matres->setTextureSampler("normalMap", normalTexRes, 2);
            matres->setTextureSampler("metallicMap", metallicTexRes, 3);
            matres->setTextureSampler("roughnessMap", roughnessTexRes, 4);
            matres->setTextureSampler("aoMap", aoTexRes, 5);

            PBRSettings settings;
            settings.useEmissiveMap = false;
            settings.emissionIntensity = 0.0f;
            matres->setUniformBuffer("Settings", &settings, sizeof(PBRSettings), 2);
        }
        {
            auto matres = resourceManager.createMaterial("pbrMaterial3");
            matres->loadFromManagedResource(renderer.getDeviceManager().createMaterial(nullptr));

            matres->setShader(shaderRes);

//            // PBR test textures
//            auto albedoImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/metalgrid/albedo.png");
//            albedoImage->load();
//            auto normalImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/metalgrid/normal.png");
//            normalImage->load();
//            auto metallicImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/metalgrid/metallic.png");
//            metallicImage->load();
//            auto roughnessImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/metalgrid/roughness.png");
//            roughnessImage->load();
//            auto aoImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/metalgrid/ao.png");
//            aoImage->load();

            // PBR test textures
            auto albedoImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/metalgrid/Sci-fi_Wall_011_basecolor.jpg");
            albedoImage->load();
            auto normalImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/metalgrid/Sci-fi_Wall_011_normal.jpg");
            normalImage->load();
            auto metallicImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/metalgrid/Sci-fi_Wall_011_metallic.jpg");
            metallicImage->load();
            auto roughnessImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/metalgrid/Sci-fi_Wall_011_roughness.jpg");
            roughnessImage->load();
            auto aoImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/metalgrid/Sci-fi_Wall_011_ambientOcclusion.jpg");
            aoImage->load();
            auto emissiveImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/metalgrid/Sci-fi_Wall_011_emissive.jpg");
            emissiveImage->load();

            // create textures from image data
            auto samplerState = renderer.getDevice().createSamplerState(SamplerStateDesc::newLinear());

            TextureDesc albedoTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, albedoImage->getWidth(), albedoImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto albedoTex = renderer.getDeviceManager().getDevice().createTexture(albedoTexDesc);
            albedoTex->upload(albedoImage->getData(), TextureRangeDesc::new2D(0, 0, albedoImage->getWidth(), albedoImage->getHeight()));
            auto albedoTexRes = resourceManager.createTexture("metalgrid/albedoMap");
            albedoTexRes->loadFromManagedResource(albedoTex, samplerState);

            TextureDesc normalTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, normalImage->getWidth(), normalImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto normalTex = renderer.getDeviceManager().getDevice().createTexture(normalTexDesc);
            normalTex->upload(normalImage->getData(), TextureRangeDesc::new2D(0, 0, normalImage->getWidth(), normalImage->getHeight()));
            auto normalTexRes = resourceManager.createTexture("metalgrid/normalMap");
            normalTexRes->loadFromManagedResource(normalTex, samplerState);

            TextureDesc metallicTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, metallicImage->getWidth(), metallicImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto metallicTex = renderer.getDeviceManager().getDevice().createTexture(metallicTexDesc);
            metallicTex->upload(metallicImage->getData(), TextureRangeDesc::new2D(0, 0, metallicImage->getWidth(), metallicImage->getHeight()));
            auto metallicTexRes = resourceManager.createTexture("metalgrid/metallicMap");
            metallicTexRes->loadFromManagedResource(metallicTex, samplerState);

            TextureDesc roughnessTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, roughnessImage->getWidth(), roughnessImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto roughnessTex = renderer.getDeviceManager().getDevice().createTexture(roughnessTexDesc);
            roughnessTex->upload(roughnessImage->getData(), TextureRangeDesc::new2D(0, 0, roughnessImage->getWidth(), roughnessImage->getHeight()));
            auto roughnessTexRes = resourceManager.createTexture("metalgrid/roughnessMap");
            roughnessTexRes->loadFromManagedResource(roughnessTex, samplerState);

            TextureDesc aoTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, aoImage->getWidth(), aoImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto aoTex = renderer.getDeviceManager().getDevice().createTexture(aoTexDesc);
            aoTex->upload(aoImage->getData(), TextureRangeDesc::new2D(0, 0, aoImage->getWidth(), aoImage->getHeight()));
            auto aoTexRes = resourceManager.createTexture("metalgrid/aoMap");
            aoTexRes->loadFromManagedResource(aoTex, samplerState);

            TextureDesc emissiveTexDesc = TextureDesc::new2D(TextureFormat::RGBA_UNorm8, emissiveImage->getWidth(), emissiveImage->getHeight(), TextureDesc::TextureUsageBits::Sampled);
            auto emissiveTex = renderer.getDeviceManager().getDevice().createTexture(emissiveTexDesc);
            emissiveTex->upload(emissiveImage->getData(), TextureRangeDesc::new2D(0, 0, emissiveImage->getWidth(), emissiveImage->getHeight()));
            auto emissiveTexRes = resourceManager.createTexture("metalgrid/emissiveMap");
            emissiveTexRes->loadFromManagedResource(emissiveTex, samplerState);

            // set textures to material
            matres->setTextureSampler("albedoMap", albedoTexRes, 1);
            matres->setTextureSampler("normalMap", normalTexRes, 2);
            matres->setTextureSampler("metallicMap", metallicTexRes, 3);
            matres->setTextureSampler("roughnessMap", roughnessTexRes, 4);
            matres->setTextureSampler("aoMap", aoTexRes, 5);
            matres->setTextureSampler("emissiveMap", emissiveTexRes, 6);

            PBRSettings settings;
            settings.useEmissiveMap = true;
            matres->setUniformBuffer("Settings", &settings, sizeof(PBRSettings), 2);
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

        auto shaderRes = resourceManager.createShader("normalShader");
        shaderRes->loadFromManagedResource(renderer.getDeviceManager().createShaderProgram(vs, fs));

        auto matres = resourceManager.createMaterial("normalMaterial");
        matres->loadFromManagedResource(renderer.getDeviceManager().createMaterial(nullptr));

        matres->setShader(shaderRes);
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

        auto shaderRes = resourceManager.createShader("testShader");
        shaderRes->loadFromManagedResource(renderer.getDeviceManager().createShaderProgram(vs, fs));

        auto matres = resourceManager.createMaterial("testMaterial");
        matres->loadFromManagedResource(renderer.getDeviceManager().createMaterial(nullptr));

        matres->setShader(shaderRes);
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

        auto shaderRes = resourceManager.createShader("floorShader");
        shaderRes->loadFromManagedResource(renderer.getDeviceManager().createShaderProgram(vs, fs));

        auto matres = resourceManager.createMaterial("floorMaterial");
        matres->loadFromManagedResource(renderer.getDeviceManager().createMaterial(nullptr));

        matres->setShader(shaderRes);
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

        auto shaderRes = resourceManager.createShader("portalShader");
        shaderRes->loadFromManagedResource(renderer.getDeviceManager().createShaderProgram(vs, fs));

        auto matres = resourceManager.createMaterial("portalMaterial");
        matres->loadFromManagedResource(renderer.getDeviceManager().createMaterial(nullptr));

        matres->setShader(shaderRes);
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
        loadObj(desc.assetPath + "/test/teapot.obj", *m);
//        objl::Loader loader;
//        bool success = loader.LoadFile(desc.assetPath + "/test/teapot.obj");
//        if (!success)
//        {
//            std::cerr << "Failed to load model" << std::endl;
//            return;
//        }
//        for (auto& LoadedVertice: loader.LoadedVertices)
//        {
//            Mesh::Vertex vertex{};
//            vertex.position = {LoadedVertice.Position.X, LoadedVertice.Position.Y, LoadedVertice.Position.Z};
//            vertex.normal = {LoadedVertice.Normal.X, LoadedVertice.Normal.Y, LoadedVertice.Normal.Z};
//            vertex.texCoords = {LoadedVertice.TextureCoordinate.X, LoadedVertice.TextureCoordinate.Y};
//            m->vertices.push_back(vertex);
//        }
//        for (unsigned int LoadedIndice: loader.LoadedIndices)
//        {
//            m->indices.push_back(LoadedIndice);
//        }
//        m->normalize();
        m->recalculateBounds();
    }
    {
        auto teapotMeshRes = resourceManager.createMesh("teapot");
        teapotMeshRes->getMesh() = *m;

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
        loadObj(desc.assetPath + "/test/cow.obj", *spiderMesh);
//        objl::Loader loader;
//        bool success = loader.LoadFile(desc.assetPath + "/test/cow.obj");
//        if (!success)
//        {
//            std::cerr << "Failed to load model" << std::endl;
//            return;
//        }
//        for (auto& LoadedVertice: loader.LoadedVertices)
//        {
//            Mesh::Vertex vertex{};
//            vertex.position = {LoadedVertice.Position.X, LoadedVertice.Position.Y, LoadedVertice.Position.Z};
//            vertex.normal = {LoadedVertice.Normal.X, LoadedVertice.Normal.Y, LoadedVertice.Normal.Z};
//            vertex.texCoords = {LoadedVertice.TextureCoordinate.X, LoadedVertice.TextureCoordinate.Y};
//            spiderMesh->vertices.push_back(vertex);
//        }
//        for (unsigned int LoadedIndice: loader.LoadedIndices)
//        {
//            spiderMesh->indices.push_back(LoadedIndice);
//        }
//        spiderMesh->normalize();
        spiderMesh->recalculateBounds();
    }
    {
        auto spiderMeshRes = resourceManager.createMesh("spider");
        spiderMeshRes->getMesh() = *spiderMesh;

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
        loadObj(desc.assetPath + "/test/suzanne.obj", *bunnyMesh);
//        objl::Loader loader;
//        bool success = loader.LoadFile(desc.assetPath + "/test/suzanne.obj");
//        if (!success)
//        {
//            std::cerr << "Failed to load model" << std::endl;
//            //return;
//        }
//        for (auto& LoadedVertice: loader.LoadedVertices)
//        {
//            Mesh::Vertex vertex{};
//            vertex.position = {LoadedVertice.Position.X, LoadedVertice.Position.Y, LoadedVertice.Position.Z};
//            vertex.normal = {LoadedVertice.Normal.X, LoadedVertice.Normal.Y, LoadedVertice.Normal.Z};
//            vertex.texCoords = {LoadedVertice.TextureCoordinate.X, LoadedVertice.TextureCoordinate.Y};
//            bunnyMesh->vertices.push_back(vertex);
//        }
//        for (unsigned int LoadedIndice: loader.LoadedIndices)
//        {
//            bunnyMesh->indices.push_back(LoadedIndice);
//        }
//        bunnyMesh->normalize();
        bunnyMesh->recalculateBounds();
    }
    {
        auto bunnyMeshRes = resourceManager.createMesh("bunny");
        bunnyMeshRes->getMesh() = *bunnyMesh;

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
                                                        { "inTexCoords", 2, VertexAttributeFormat::Float2 },
//                { "inTangents", 3, VertexAttributeFormat::Float3 },
//                { "inBitangents", 4, VertexAttributeFormat::Float3 }
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
                                                        { "inTexCoords", 2, VertexAttributeFormat::Float2 },
//                { "inTangents", 3, VertexAttributeFormat::Float3 },
//                { "inBitangents", 4, VertexAttributeFormat::Float3 }
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
                                                        { "inTexCoords", 2, VertexAttributeFormat::Float2 },
//                { "inTangents", 3, VertexAttributeFormat::Float3 },
//                { "inBitangents", 4, VertexAttributeFormat::Float3 }
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

    // skybox material
    {
        auto vs = R"(
            #version 450
            layout(location = 0) in vec3 inPosition;

            layout(location = 0) out vec3 fragTexCoord;

            layout(binding = 0) uniform UBO {
                mat4 model;
                mat4 view;
                mat4 proj;
            } ubo;

            void main() {
                gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
                fragTexCoord = inPosition;
            }
        )";

        auto fs = R"(
            #version 450
            layout(location = 0) in vec3 fragTexCoord;
            out vec4 fragColor;

            layout(binding = 1) uniform samplerCube skybox;

            void main() {
                // convert to linear
                vec3 color = texture(skybox, fragTexCoord).rgb;
                color = pow(color, vec3(2.2));
                fragColor = vec4(color, 1.0);
            }
        )";

        auto shaderRes = resourceManager.createShader("skyboxShader");
        shaderRes->loadFromManagedResource(renderer.getDeviceManager().createShaderProgram(vs, fs));

        auto matres = resourceManager.createMaterial("skyboxMaterial");
        matres->loadFromManagedResource(renderer.getDeviceManager().createMaterial(nullptr));
        matres->getMaterial()->setDepthTestConfig(graphics::DepthTestConfig::Disable);

        matres->setShader(shaderRes);
    }

    // Load skybox texture
    {
        auto rightImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/skybox/right.jpg");
        rightImage->load();
        auto leftImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/skybox/left.jpg");
        leftImage->load();
        auto topImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/skybox/top.jpg");
        topImage->load();
        auto bottomImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/skybox/bottom.jpg");
        bottomImage->load();
        auto frontImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/skybox/front.jpg");
        frontImage->load();
        auto backImage = resourceManager.createExternalImage(desc.assetPath + "/test/textures/skybox/back.jpg");
        backImage->load();

        auto faceTexRange = TextureRangeDesc::new2D(0, 0, rightImage->getWidth(), rightImage->getHeight());

        auto cubeTex = renderer.getDevice().createTexture(TextureDesc::newCube(TextureFormat::RGBA_UNorm8, rightImage->getWidth(), rightImage->getHeight(), TextureDesc::TextureUsageBits::Sampled));
        cubeTex->uploadCube(rightImage->getData(), TextureCubeFace::PosX, faceTexRange, 0);
        cubeTex->uploadCube(leftImage->getData(), TextureCubeFace::NegX, faceTexRange, 0);
        cubeTex->uploadCube(topImage->getData(), TextureCubeFace::PosY, faceTexRange, 0);
        cubeTex->uploadCube(bottomImage->getData(), TextureCubeFace::NegY, faceTexRange, 0);
        cubeTex->uploadCube(frontImage->getData(), TextureCubeFace::PosZ, faceTexRange, 0);
        cubeTex->uploadCube(backImage->getData(), TextureCubeFace::NegZ, faceTexRange, 0);

        auto skyboxTexRes = resourceManager.createTexture("skybox");
        skyboxTexRes->loadFromManagedResource(cubeTex, renderer.getDevice().createSamplerState(SamplerStateDesc::newLinear()));

        // unload images
        rightImage->unload();
        leftImage->unload();
        topImage->unload();
        bottomImage->unload();
        frontImage->unload();
        backImage->unload();
    }

    // create skybox mesh
    {
        auto skyboxMesh = Mesh::createCube(10.0f);
        auto skyboxMeshRes = resourceManager.createMesh("skyboxMesh");
        skyboxMeshRes->getMesh() = *skyboxMesh;

        graphics::VertexDataLayout attribLayout({
                                                        { "inPosition", 0, VertexAttributeFormat::Float3 },
                                                        { "inNormal", 1, VertexAttributeFormat::Float3 }, // not used but needed for padding rn
                                                        { "inTexCoords", 2, VertexAttributeFormat::Float2 } // ^
                                                });

        auto vertexData = renderer.getDeviceManager().createVertexData(attribLayout);
        vertexData->allocateVertexBuffer(renderer.getDevice(), skyboxMesh->vertices.size());
        vertexData->allocateIndexBuffer(renderer.getDevice(), skyboxMesh->indices.size());
        vertexData->pushVertices(skyboxMesh->vertices);
        vertexData->pushIndices(skyboxMesh->indices);

        skyboxMeshRes->setVertexData(vertexData);
        skyboxMeshRes->load();
    }

    // create procedural texture via compute shader
    {
        auto cs = R"(
            #version 450
            layout(local_size_x = 16, local_size_y = 16) in;

            layout(binding = 0, rgba8) uniform writeonly image2D img;

            float rand(vec2 co)
            {
                return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
            }

            void main() {
                ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
                float Resolution = 1024.0;
                float x = 10000 * (gl_GlobalInvocationID.x-Resolution/2)/Resolution;
                float y = 10000 * (gl_GlobalInvocationID.y-Resolution/2)/Resolution;

                float distance = sqrt(pow(x, 2)+pow(y, 2));
                float angle = atan((x)/(y));
                float core = 1 - pow(distance/200, 2);
                float arms = exp(-distance/1500) * 0.5 * pow(sin(pow(0.5 * distance, 0.35) - angle), 2) + 0.5f - distance/10000;
                float density = max(core, max(arms, 0));//*(1f-Mathf.Pow(Mathf.Abs(z/50f)*(0.5f + distance/10000), 1.5f));

                vec3 color = vec3(density);

                imageStore(img, pixelCoords, vec4(color, 1.0));
            }
        )";
        auto computeShader = renderer.getDevice().createShaderModule({.type = ShaderModuleType::Compute,
                                                                             .code = cs});
        auto computeShaderStage = renderer.getDevice().createPipelineShaderStages(PipelineShaderStagesDesc::fromComputeModule(computeShader));

        auto pipeline = renderer.getDevice().createComputePipeline({
                                                                           .shaderStages = computeShaderStage,
                                                                           .imagesMap = {
                                                                                   {0, "img"}
                                                                           }
                                                                   });

        int width = 1024;
        int height = 1024;
        auto renderTex = renderer.getDevice().createTexture(TextureDesc::new2D(TextureFormat::RGBA_UNorm8, width, height, TextureDesc::TextureUsageBits::Sampled | TextureDesc::TextureUsageBits::Attachment | TextureDesc::TextureUsageBits::Storage));

        auto cmdPool = renderer.getDevice().createCommandPool({});

        auto cmdBuffer = cmdPool->acquireComputeCommandBuffer({});
        cmdBuffer->begin();

        cmdBuffer->bindComputePipeline(pipeline);
        cmdBuffer->bindImage(0, renderTex, ReadWrite);
        cmdBuffer->dispatch({static_cast<uint32_t>(renderTex->getWidth() / 16), static_cast<uint32_t>(renderTex->getHeight() / 16), 1});

        cmdBuffer->end();
        cmdPool->submitCommandBuffer(std::move(cmdBuffer));

        auto texRes = resourceManager.createTexture("proceduralTexture");
        texRes->loadFromManagedResource(renderTex, renderer.getDevice().createSamplerState(SamplerStateDesc::newLinear()));
    }

}
}
