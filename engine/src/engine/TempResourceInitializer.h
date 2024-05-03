//
// Created by Jonathan Richard on 2024-04-30.
//

#pragma once

#include "engine/graphics/Renderer.h"
#include "engine/ResourceManager.h"
#include "engine/graphics/Material.h"
#include "engine/graphics/DeviceManager.h"



namespace TempResourceInitializer
{
    void init(ResourceManager& resourceManager, graphics::Renderer& renderer, const InstanceDesc& desc)
    {
            // Create materials

    // PBR material
    {
        auto vs = R"(
            #version 450

            const int MAX_LIGHTS = 10;


            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec3 inNormal;
            layout(location = 2) in vec2 inTexCoord;
//            layout(location = 3) in vec3 inTangent;
//            layout(location = 4) in vec3 inBitangent;

            layout(location = 0) out vec2 fragTexCoord;
            layout(location = 1) out vec3 fragWorldPos;
            layout(location = 2) out vec3 fragNormal;

            // normal mapping
//            layout(location = 3) out vec3 tangentViewPos;
//            layout(location = 4) out vec3 tangentLightPos[10];
//            layout(location = 5) out vec3 tangentFragPos;

            struct Light {
                vec3 position;
                vec3 color;
                float intensity;
                float constant;
                float linear;
                float quadratic;
            };

            struct Spotlight {
                vec3 position;
                vec3 direction;
                vec3 color;

                float cutoff;
                float outerCutoff;

                float intensity;
                float constant;
                float linear;
                float quadratic;
            };

            layout(binding = 6, std140) uniform Lights {
                int lightCount;
                Light lights[MAX_LIGHTS];
            } pointLights;


            layout(binding = 7, std140) uniform Spotlights {
                int lightCount;
                Spotlight lights[MAX_LIGHTS];
            } spotlights;

            struct DirectionalLight {
                vec3 color;
                vec3 direction;
                float intensity;
            };

            layout(binding = 8, std140) uniform DirectionalLights {
                DirectionalLight directionalLight;
            } directionalLights;


            layout(binding = 1) uniform Constants {
                vec3 cameraPos;
                vec3 cameraDir;
                vec3 unused_;
                int lightModel;
            } constants;

            layout(binding = 2, std140) uniform Settings {
                bool useAlbedoMap;
                bool useNormalMap;
                bool useMetallicMap;
                bool useRoughnessMap;
                bool useAOMap;
                bool useEmissiveMap;

                vec3 baseColor;
                float metallic;
                float roughness;
                float ao;
                vec3 emissionColor;
                float emissionIntensity;

                int lightModel; // 0 = PBR, 1 = Blinn-Phong, 2 = Gouraud
            } settings;

            layout(binding = 0) uniform UBO {
                mat4 model;
                mat4 view;
                mat4 proj;
            } ubo;

            layout(location = 3) out vec3 gouraudColor;

            vec3 calcGouraud(vec3 N, vec3 V, vec3 L, vec3 color, vec3 albedo, float intensity)
            {
                vec3 lightDir = normalize(L);
                vec3 normal = normalize(N);
                float diff = max(dot(lightDir, normal), 0.0);
                vec3 diffuse = albedo * diff * color;

                vec3 viewDir = normalize(V);
                vec3 halfwayDir = normalize(lightDir + viewDir);
                float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
                vec3 specular = color * spec;

                return (diffuse + specular) * intensity;
            }

            float calcAttenuation(float distance, float constant, float linear, float quadratic)
            {
                return 1.0 / (constant + linear * distance + quadratic * distance * distance);
            }

            float calcPointLightIntensity(vec3 lightPos, vec3 fragPos, float constant, float linear, float quadratic)
            {
                float distance = length(lightPos - fragPos);
                float attenuation = calcAttenuation(distance, constant, linear, quadratic);
                return attenuation;
            }

            float calcSpotlightIntensity(vec3 lightPos, vec3 spotDir, vec3 fragPos, float constant, float linear, float quadratic, float cutoff, float outerCutoff)
            {
                float distance = length(lightPos - fragPos);
                float attenuation = calcAttenuation(distance, constant, linear, quadratic);
                vec3 lightDir = normalize(lightPos - fragPos);
                float theta = dot(lightDir, normalize(-spotDir));
                float epsilon = cutoff - outerCutoff;
                float intensity = clamp((theta - outerCutoff) / epsilon, 0.0, 1.0) * attenuation;
                return intensity;
            }

            void main() {
                gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
                fragTexCoord = inTexCoord;
                fragWorldPos = vec3(ubo.model * vec4(inPosition, 1.0));
                mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
                fragNormal = normalMatrix * inNormal;

                if (constants.lightModel == 3) {
                    vec3 albedo =  settings.baseColor;
                    vec3 emission = settings.emissionColor * settings.emissionIntensity;

                    vec3 fragNormal = normalize(fragNormal);
                    vec3 fragWorldPos = vec3(ubo.model * vec4(inPosition, 1.0));

                    vec3 Lo = vec3(0.0);
                    for (int i = 0; i < 1; ++i) {
                        Lo += calcGouraud(fragNormal, constants.cameraPos - fragWorldPos, directionalLights.directionalLight.direction, directionalLights.directionalLight.color, albedo, directionalLights.directionalLight.intensity);
                    }

                    for (int i = 0; i < pointLights.lightCount; ++i) {
                        Light light = pointLights.lights[i];
                        float intensity = calcPointLightIntensity(light.position, fragWorldPos, light.constant, light.linear, light.quadratic);
                        Lo += calcGouraud(fragNormal, constants.cameraPos - fragWorldPos, light.position - fragWorldPos, light.color, albedo, light.intensity) * intensity;
                    }

                    for (int i = 0; i < spotlights.lightCount; ++i) {
                        Spotlight light = spotlights.lights[i];
                        float intensity = calcSpotlightIntensity(light.position, light.direction, fragWorldPos, light.constant, light.linear, light.quadratic, light.cutoff, light.outerCutoff);
                        Lo += calcGouraud(fragNormal, constants.cameraPos - fragWorldPos, light.position - fragWorldPos, light.color, albedo, light.intensity) * intensity;
                    }
                    vec3 ambient = vec3(0.00) * albedo;

                    gouraudColor = ambient + emission + Lo;
                }

            }
        )";

        auto fs = R"(
            #version 450

            const int MAX_LIGHTS = 10;

            layout(location = 0) in vec2 fragTexCoord;
            layout(location = 1) in vec3 fragWorldPos;
            layout(location = 2) in vec3 fragNormal;
            layout(location = 3) in vec3 gouraudColor;

            out vec4 fragColor;

            layout(binding = 1) uniform sampler2D albedoMap;
            layout(binding = 2) uniform sampler2D normalMap;
            layout(binding = 3) uniform sampler2D metallicMap;
            layout(binding = 4) uniform sampler2D roughnessMap;
            layout(binding = 5) uniform sampler2D aoMap;
            layout(binding = 6) uniform sampler2D emissiveMap;

            struct Material {
                vec3 albedo;
                float metallic;
                float roughness;
                float ao;
            };

            struct Light {
                vec3 position;
                vec3 color;
                float intensity;
                float constant;
                float linear;
                float quadratic;
            };

            struct Spotlight {
                vec3 position;
                vec3 direction;
                vec3 color;

                float cutoff;
                float outerCutoff;

                float intensity;
                float constant;
                float linear;
                float quadratic;
            };

            layout(binding = 6, std140) uniform Lights {
                int lightCount;
                Light lights[MAX_LIGHTS];
            } pointLights;


            layout(binding = 7, std140) uniform Spotlights {
                int lightCount;
                Spotlight lights[MAX_LIGHTS];
            } spotlights;

            struct DirectionalLight {
                vec3 color;
                vec3 direction;
                float intensity;
            };

            layout(binding = 8, std140) uniform DirectionalLights {
                DirectionalLight directionalLight;
            } directionalLights;


            layout(binding = 1) uniform Constants {
                vec3 cameraPos;
                vec3 cameraDir;
                vec3 unused_;
                int lightModel;
            } constants;

            layout(binding = 2, std140) uniform Settings {
                bool useAlbedoMap;
                bool useNormalMap;
                bool useMetallicMap;
                bool useRoughnessMap;
                bool useAOMap;
                bool useEmissiveMap;

                vec3 baseColor;
                float metallic;
                float roughness;
                float ao;
                vec3 emissionColor;
                float emissionIntensity;

                int lightModel; // 0 = PBR, 1 = Blinn-Phong, 2 = Gouraud
            } settings;

            const float PI = 3.14159265359;

            mat3 cotangentFrame(vec3 N, vec3 p, vec2 uv)
            {
                vec3 dp1 = dFdx(p);
                vec3 dp2 = dFdy(p);
                vec2 duv1 = dFdx(uv);
                vec2 duv2 = dFdy(uv);

                vec3 dp2perp = cross(dp2, N);
                vec3 dp1perp = cross(N, dp1);
                vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
                vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

                float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
                return mat3(T * invmax, B * invmax, N);
            }

            vec3 perturbNormal(vec3 N, vec3 V, vec2 uv)
            {
                vec3 map = texture(normalMap, uv).xyz;
                map = map * 255./127. - 128./127.;
                mat3 TBN = cotangentFrame(N, -V, uv);

//                map.y = -map.y;

                return normalize(TBN * map);
            }

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


            vec3 calcCookTorrance(vec3 N, vec3 V, vec3 L, vec3 F0, vec3 albedo, float roughness, float metallic)
            {
                N = normalize(N);
                V = normalize(V);
                L = normalize(L);

                vec3 H = normalize(V + L);
                float NDF = DistributionGGX(N, H, roughness);
                float G = GeometrySmith(N, V, L, roughness);
                vec3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

                vec3 kS = F;
                vec3 kD = vec3(1.0) - kS;
                kD *= 1.0 - metallic;

                vec3 numerator = NDF * G * F;
                float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
                vec3 specular = numerator / denominator;

                float NdotL = max(dot(N, L), 0.0);

                return (kD * albedo / PI + specular) * NdotL;
            }

            vec3 calcBlinnPhong(vec3 N, vec3 V, vec3 L, vec3 color, vec3 albedo, float intensity, float shininess)
            {
                vec3 lightDir = normalize(L);
                vec3 normal = normalize(N);
                float diff = max(dot(lightDir, normal), 0.0);
                vec3 diffuse = albedo * diff * color;

                vec3 viewDir = normalize(V);
                vec3 halfwayDir = normalize(lightDir + viewDir);
                float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
                vec3 specular = color * spec;

                return (diffuse + specular) * intensity;
            }

            vec3 calculatePhong(vec3 N, vec3 V, vec3 L, vec3 color, vec3 albedo, float intensity, float shininess)
            {
                vec3 lightDir = normalize(L);
                vec3 normal = normalize(N);
                float diff = max(dot(lightDir, normal), 0.0);
                vec3 diffuse = albedo * diff * color;

                vec3 viewDir = normalize(V);
                vec3 reflectDir = reflect(-lightDir, normal);
                float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
                vec3 specular = color * spec;

                return (diffuse + specular) * intensity;
            }

            float calcAttenuation(float distance, float constant, float linear, float quadratic)
            {
                return 1.0 / (constant + linear * distance + quadratic * distance * distance);
            }

            float calcPointLightIntensity(vec3 lightPos, vec3 fragPos, float constant, float linear, float quadratic)
            {
                float distance = length(lightPos - fragPos);
                float attenuation = calcAttenuation(distance, constant, linear, quadratic);
                return attenuation;
            }

            float calcSpotlightIntensity(vec3 lightPos, vec3 spotDir, vec3 fragPos, float constant, float linear, float quadratic, float cutoff, float outerCutoff)
            {
                float distance = length(lightPos - fragPos);
                float attenuation = calcAttenuation(distance, constant, linear, quadratic);
                vec3 lightDir = normalize(lightPos - fragPos);
                float theta = dot(lightDir, normalize(-spotDir));
                float epsilon = cutoff - outerCutoff;
                float intensity = clamp((theta - outerCutoff) / epsilon, 0.0, 1.0) * attenuation;
                return intensity;
            }

            void main() {

                vec3 camPos = constants.cameraPos;
                vec3 fragNormal = normalize(fragNormal);

                vec3 albedo =  settings.useAlbedoMap ? pow(texture(albedoMap, fragTexCoord).rgb, vec3(2.2)) : vec3(1.0);
                albedo = albedo * settings.baseColor;

                vec3 normal = settings.useNormalMap ? perturbNormal(fragNormal, camPos - fragWorldPos, fragTexCoord) : fragNormal;
                float metallic = settings.useMetallicMap ? texture(metallicMap, fragTexCoord).r : settings.metallic;
                float roughness = settings.useRoughnessMap ? texture(roughnessMap, fragTexCoord).r : settings.roughness;
                float ao = settings.useAOMap ? texture(aoMap, fragTexCoord).r : settings.ao;
                vec3 emission = settings.useEmissiveMap ? texture(emissiveMap, fragTexCoord).rgb : vec3(1.0);
                emission = emission * clamp(settings.emissionColor, 0.0, 1.0) * settings.emissionIntensity;

                Material material = Material(albedo, metallic, roughness, ao);


                vec3 V = normalize(camPos - fragWorldPos);
                vec3 N = normal;

                vec3 color = vec3(0.0);

                if (constants.lightModel == 0) {
                    vec3 F0 = vec3(0.04);
                    F0 = mix(F0, albedo, metallic);

                    vec3 Lo = vec3(0.0);

                    for (int i = 0; i < 1; ++i) {
                        Lo += calcCookTorrance(N, V, directionalLights.directionalLight.direction, F0, material.albedo, material.roughness, material.metallic) * directionalLights.directionalLight.color * directionalLights.directionalLight.intensity;
                    }

                    for (int i = 0; i < pointLights.lightCount; ++i) {
                        Light light = pointLights.lights[i];
                        float intensity = calcPointLightIntensity(light.position, fragWorldPos, light.constant, light.linear, light.quadratic);
                        Lo += calcCookTorrance(N, V, light.position - fragWorldPos, F0, material.albedo, material.roughness, material.metallic) * light.color * intensity * light.intensity;
                    }

                    for (int i = 0; i < spotlights.lightCount; ++i) {
                        Spotlight light = spotlights.lights[i];
                        float intensity = calcSpotlightIntensity(light.position, light.direction, fragWorldPos, light.constant, light.linear, light.quadratic, light.cutoff, light.outerCutoff);
                        Lo += calcCookTorrance(N, V, light.position - fragWorldPos, F0, material.albedo, material.roughness, material.metallic) * light.color * intensity * light.intensity;
                    }

                    vec3 ambient = vec3(0.00) * material.albedo * material.ao;

                    color = ambient + Lo;

                    color = color / (color + vec3(1.0));
                    color = pow(color, vec3(1.0 / 2.2));

                    color += emission;
                }
                else if (constants.lightModel == 1) {
                    vec3 Lo = vec3(0.0);

                    for (int i = 0; i < 1; ++i) {
                        Lo += calcBlinnPhong(N, V, directionalLights.directionalLight.direction, directionalLights.directionalLight.color, material.albedo, directionalLights.directionalLight.intensity, 32.0);
                    }

                    for (int i = 0; i < pointLights.lightCount; ++i) {
                        Light light = pointLights.lights[i];
                        float intensity = calcPointLightIntensity(light.position, fragWorldPos, light.constant, light.linear, light.quadratic);
                        Lo += calcBlinnPhong(N, V, light.position - fragWorldPos, light.color, material.albedo, light.intensity, 32.0) * intensity;
                    }

                    for (int i = 0; i < spotlights.lightCount; ++i) {
                        Spotlight light = spotlights.lights[i];
                        float intensity = calcSpotlightIntensity(light.position, light.direction, fragWorldPos, light.constant, light.linear, light.quadratic, light.cutoff, light.outerCutoff);
                        Lo += calcBlinnPhong(N, V, light.position - fragWorldPos, light.color, material.albedo, light.intensity, 32.0) * intensity;
                    }

                    vec3 ambient = vec3(0.00) * material.albedo * material.ao;

                    color = ambient + Lo;

//                    color = color / (color + vec3(1.0));
//                    color = pow(color, vec3(1.0 / 2.2));

                    color += emission;
                }
                else if (constants.lightModel == 2) {
                    vec3 Lo = vec3(0.0);

                    for (int i = 0; i < 1; ++i) {
                        Lo += calculatePhong(N, V, directionalLights.directionalLight.direction, directionalLights.directionalLight.color, material.albedo, directionalLights.directionalLight.intensity, 32.0);
                    }

                    for (int i = 0; i < pointLights.lightCount; ++i) {
                        Light light = pointLights.lights[i];
                        float intensity = calcPointLightIntensity(light.position, fragWorldPos, light.constant, light.linear, light.quadratic);
                        Lo += calculatePhong(N, V, light.position - fragWorldPos, light.color, material.albedo, light.intensity, 32.0) * intensity;
                    }

                    for (int i = 0; i < spotlights.lightCount; ++i) {
                        Spotlight light = spotlights.lights[i];
                        float intensity = calcSpotlightIntensity(light.position, light.direction, fragWorldPos, light.constant, light.linear, light.quadratic, light.cutoff, light.outerCutoff);
                        Lo += calculatePhong(N, V, light.position - fragWorldPos, light.color, material.albedo, light.intensity, 32.0) * intensity;
                    }

                    vec3 ambient = vec3(0.00) * material.albedo * material.ao;

                    color = ambient + Lo;
                    color += emission;
                }
                else if (constants.lightModel == 3) {
                    color = gouraudColor;// * material.albedo;
                }



                color.r = max(color.r, 0.0);
                color.g = max(color.g, 0.0);
                color.b = max(color.b, 0.0);

                fragColor = vec4(color, 1.0);
            }
        )";

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

    }
}