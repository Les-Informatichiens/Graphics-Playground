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


    // screen quad vertex buffer
    {
        auto vs = R"(
            #version 450

            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec2 inTexCoords;

            layout(location = 0) out vec2 fragTexCoord;

            void main() {
                gl_Position = vec4(inPosition, 1.0);
                fragTexCoord = inTexCoords;
            }
        )";

        auto fs = R"(
            #version 450

            layout(location = 0) in vec2 fragTexCoord;
            out vec4 frag_color;

            layout(binding = 0) uniform sampler2D screenTexture;

            layout(binding = 0) uniform Settings {
                float u_exposure;
                float u_gamma;
            };

            vec3 gammaCorrect(vec3 color)
            {
                return pow(color, vec3(1.0/u_gamma));
            }

            // sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
            mat3 ACESInputMat =
            {
                {0.59719, 0.07600, 0.02840},
                {0.35458, 0.90834, 0.13383},
                {0.04823, 0.01566, 0.83777}
            };

            // ODT_SAT => XYZ => D60_2_D65 => sRGB
            mat3 ACESOutputMat =
            {
                { 1.60475, -0.10208, -0.00327},
                {-0.53108,  1.10813, -0.07276},
                {-0.07367, -0.00605,  1.07602 }
            };

            vec3 RRTAndODTFit(vec3 v)
            {
                vec3 a = v * (v + 0.0245786f) - 0.000090537f;
                vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
                return a / b;
            }

            void main()
            {
                vec4 x = u_exposure * texture(screenTexture, fragTexCoord);

                vec3 color = ACESInputMat * x.rgb;
                     color = RRTAndODTFit(color);
                     color = ACESOutputMat * color;

                     color = gammaCorrect(color);
                     color = clamp(color, 0.0, 1.0);

                frag_color = vec4(color, 1.0);
            }
        )";

        auto shaderRes = resourceManager.createShader("screenQuadShader");
        shaderRes->loadFromManagedResource(renderer.getDeviceManager().createShaderProgram(vs, fs));

        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 0.0f, 1.0f, 1.0f
        };
        graphics::VertexDataLayout attribLayout({
                { "inPosition", 0, VertexAttributeFormat::Float3 },
                { "inTexCoords", 1, VertexAttributeFormat::Float2 }
        });
        screenQuadBuffer = renderer.getDeviceManager().createVertexData(attribLayout, 6);
        screenQuadBuffer->pushVertices(quadVertices, 6);

        auto matres = resourceManager.createMaterial("screenQuadMaterial");
        matres->loadFromManagedResource(renderer.getDeviceManager().createMaterial(nullptr));
        matres->setShader(shaderRes);
    }

    // ----------- Initializing HDR FBO ------------
    {
        auto colorBufferDesc = TextureDesc::new2D(
                TextureFormat::RGBA_F32,
                desc.width,
                desc.height,
                TextureDesc::TextureUsageBits::Attachment | TextureDesc::TextureUsageBits::Sampled | TextureDesc::TextureUsageBits::Storage);
        colorBufferDesc.numMipLevels = calculateMipmapLevels(desc.width, desc.height);
                //TextureDesc::calcNumMipLevels(desc.width, desc.height);
        hdrColorBuffers = {
                renderer.getDevice().createTexture(colorBufferDesc),
                renderer.getDevice().createTexture(colorBufferDesc)
        };

        hdrSampler = renderer.getDevice().createSamplerState(SamplerStateDesc::newLinearMipmapped());

        auto hdrTexRes = resourceManager.createTexture("hdrColorBuffer0");
        hdrTexRes->loadFromManagedResource(hdrColorBuffers[0], renderer.getDevice().createSamplerState(SamplerStateDesc::newLinear()));

        hdrDepthBuffer = renderer.getDevice().createTexture(TextureDesc::new2D(
                TextureFormat::Z_UNorm24,
                desc.width,
                desc.height,
                TextureDesc::TextureUsageBits::Attachment | TextureDesc::TextureUsageBits::Sampled));

        hdrFramebuffer = renderer.getDevice().createFramebuffer({
                .colorAttachments = {
                        {0, {hdrColorBuffers[0], nullptr}},
//                        {1, {hdrColorBuffers[1], nullptr}}
                },
                .depthAttachment = {hdrDepthBuffer, nullptr}
        });
    }

    // ----------- Initializing scene objects ------------

    // Setup camera
    activeCamera->setProjectionConfig(90.0f, (float)desc.width / (float)desc.height, 0.1f, 100.0f);
//    activeCamera->getTransform().setPosition({ 0.0f, 0.0f, 20.0f });


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



            layout(binding = 0) uniform UBO {
                mat4 model;
                mat4 view;
                mat4 proj;
            } ubo;


//            struct Light {
//                vec3 position;
//                vec3 color;
//            };
//
//            layout(binding = 6) uniform Lights {
//                int lightCount;
//                Light lights[MAX_LIGHTS];
//            };

            void main() {
                gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
                fragTexCoord = inTexCoord;
                fragWorldPos = vec3(ubo.model * vec4(inPosition, 1.0));
                mat3 normalMatrix = transpose(inverse(mat3(ubo.model)));
                fragNormal = normalMatrix * inNormal;

//                // Gram-Schmidt Orthogonalisation
//                vec3 T = normalize(normalMatrix * inTangent);
//                vec3 N = normalize(normalMatrix * inNormal);
//                T = normalize(T - dot(T, N) * N);
//                vec3 B = cross(N, T);
//
//                mat3 TBN = transpose(mat3(T, B, N));
//
//                tangentViewPos = TBN * vec3(ubo.view * vec4(fragWorldPos, 1.0));
//                tangentFragPos = TBN * vec3(ubo.model * vec4(inPosition, 1.0));
//                for (int i = 0; i < lightCount; ++i) {
//                    tangentLightPos[i] = TBN * lights[i].position;
//                }
            }
        )";

        auto fs = R"(
            #version 450

            const int MAX_LIGHTS = 10;

            layout(location = 0) in vec2 fragTexCoord;
            layout(location = 1) in vec3 fragWorldPos;
            layout(location = 2) in vec3 fragNormal;
//            layout(location = 3) in vec3 tangentViewPos;
//            layout(location = 4) in vec3 tangentLightPos[10];
//            layout(location = 5) in vec3 tangentFragPos;

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
                vec3 lightDir;
                float shininess;
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

            vec3 getNormalFromMap()
            {
                vec3 tangentNormal = texture(normalMap, fragTexCoord).xyz * 2.0 - 1.0;

                vec3 Q1  = dFdx(fragWorldPos);
                vec3 Q2  = dFdy(fragWorldPos);
                vec2 st1 = dFdx(fragTexCoord);
                vec2 st2 = dFdy(fragTexCoord);

                vec3 N   = normalize(fragNormal);
                vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
                vec3 B  = -normalize(cross(N, T));
                mat3 TBN = mat3(T, B, N);

                return normalize(TBN * tangentNormal);
            }

            //http://pocket.gl/normal-maps/
            vec3 perturbNormal2( vec3 eye_pos, vec3 surf_norm, vec2 uv_coords, sampler2D tex){
                vec3 pNorm	= texture( tex, uv_coords ).rgb * 2.0 - 1.0;

                vec2 st0	= dFdx( uv_coords.st ),
                     st1	= dFdy( uv_coords.st );
                vec3 q0		= dFdx( eye_pos.xyz ),
                     q1		= dFdy( eye_pos.xyz ),
                     S		= normalize(  q0 * st1.t - q1 * st0.t ),
                     T		= normalize( -q0 * st1.s + q1 * st0.s ),
                     N		= normalize( surf_norm );

//                vec3 NfromST = cross( S, T );
//                if( dot( NfromST, N ) < 0.0 ) {
//                S *= -1.0;
//                T *= -1.0;
//                }

                return normalize( mat3( S, T, N ) * pNorm );
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

            // function that calculates the lighting for point lights
            // using the Cook-Torrance model
            // and light properties such as
            // position, color, intensity, constant, linear, quadratic
            vec3 calcPointLight(Light light, vec3 normal, vec3 viewDir, vec3 fragPos, Material material, vec3 F0)
            {
                vec3 lightDir = normalize(light.position - fragPos);
                vec3 reflectDir = reflect(-lightDir, normal);
                vec3 halfwayDir = normalize(lightDir + viewDir);

                float distance = length(light.position - fragPos);
                float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

                float NDF = DistributionGGX(normal, halfwayDir, material.roughness);
                float G = GeometrySmith(normal, viewDir, lightDir, material.roughness);
                vec3 F = fresnelSchlick(clamp(dot(halfwayDir, viewDir), 0.0, 1.0), F0);

                vec3 kS = F;
                vec3 kD = vec3(1.0) - kS;
                kD *= 1.0 - material.metallic;

                float NdotL = max(dot(normal, lightDir), 0.0);
                float NdotV = max(dot(normal, viewDir), 0.0);
                float NdotH = max(dot(normal, halfwayDir), 0.0);
                float VdotH = max(dot(viewDir, halfwayDir), 0.0);

                vec3 numerator = NDF * G * F;
                float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
                vec3 specular = numerator / denominator;

                return (kD * material.albedo / PI + specular) * light.color * light.intensity * NdotL * attenuation;
            }

            // function that calculates the lighting for spot lights
            // and spotlight properties such as
            // position, direction, color, intensity, constant, linear, quadratic, cutoff, outerCutoff
            vec3 calcSpotlight(Spotlight spotlight, vec3 normal, vec3 viewDir, vec3 fragPos, Material material, vec3 F0)
            {
                vec3 lightDir = normalize(spotlight.position - fragPos);
                vec3 reflectDir = reflect(-lightDir, normal);
                vec3 halfwayDir = normalize(lightDir + viewDir);

                float distance = length(spotlight.position - fragPos);
                float attenuation = 1.0 / (spotlight.constant + spotlight.linear * distance + spotlight.quadratic * (distance * distance));

                float theta = dot(lightDir, normalize(-spotlight.direction));
                float epsilon = spotlight.cutoff - spotlight.outerCutoff;
                float intensity = clamp((theta - spotlight.outerCutoff) / epsilon, 0.0, 1.0);

                float NDF = DistributionGGX(normal, halfwayDir, material.roughness);
                float G = GeometrySmith(normal, viewDir, lightDir, material.roughness);
                vec3 F = fresnelSchlick(clamp(dot(halfwayDir, viewDir), 0.0, 1.0), F0);

                vec3 kS = F;
                vec3 kD = vec3(1.0) - kS;
                kD *= 1.0 - material.metallic;

                float NdotL = max(dot(normal, lightDir), 0.0);
                float NdotV = max(dot(normal, viewDir), 0.0);
                float NdotH = max(dot(normal, halfwayDir), 0.0);
                float VdotH = max(dot(viewDir, halfwayDir), 0.0);

                vec3 numerator = NDF * G * F;
                float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
                vec3 specular = numerator / denominator;

                return (kD * material.albedo / PI + specular) * spotlight.color * spotlight.intensity * NdotL * attenuation * intensity;
            }

            // function that calculates the lighting for directional lights
            vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 fragPos, Material material, vec3 F0)
            {
                vec3 lightDir = normalize(-light.direction);
                vec3 reflectDir = reflect(-lightDir, normal);
                vec3 halfwayDir = normalize(lightDir + viewDir);

                float NDF = DistributionGGX(normal, halfwayDir, material.roughness);
                float G = GeometrySmith(normal, viewDir, lightDir, material.roughness);
                vec3 F = fresnelSchlick(clamp(dot(halfwayDir, viewDir), 0.0, 1.0), F0);

                vec3 kS = F;
                vec3 kD = vec3(1.0) - kS;
                kD *= 1.0 - material.metallic;

                float NdotL = max(dot(normal, lightDir), 0.0);
                float NdotV = max(dot(normal, viewDir), 0.0);
                float NdotH = max(dot(normal, halfwayDir), 0.0);
                float VdotH = max(dot(viewDir, halfwayDir), 0.0);

                vec3 numerator = NDF * G * F;
                float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
                vec3 specular = numerator / denominator;

                return (kD * material.albedo / PI + specular) * light.color * light.intensity * NdotL;
            }

            void main() {

                vec3 camPos = constants.cameraPos;
                vec3 fragNormal = normalize(fragNormal);

                vec3 albedo =  settings.useAlbedoMap ? pow(texture(albedoMap, fragTexCoord).rgb, vec3(2.2)) : vec3(1.0);
//                vec3 albedo = texture(albedoMap, fragTexCoord).rgb;
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

                vec3 F0 = vec3(0.04);
                F0 = mix(F0, albedo, metallic);

                vec3 Lo = vec3(0.0);

                for (int i = 0; i < 1; ++i) {
                    Lo += calcDirLight(directionalLights.directionalLight, N, V, fragWorldPos, material, F0);
                }

                for (int i = 0; i < pointLights.lightCount; ++i) {
                    Lo += calcPointLight(pointLights.lights[i], N, V, fragWorldPos, material, F0);
                }

                for (int i = 0; i < spotlights.lightCount; ++i) {
                    Lo += calcSpotlight(spotlights.lights[i], N, V, fragWorldPos, material, F0);
                }

                vec3 ambient = vec3(0.00) * material.albedo * material.ao;

                vec3 color = ambient + Lo;

                color = color / (color + vec3(1.0));
                color = pow(color, vec3(1.0 / 2.2));

                color += emission;

                color.r = max(color.r, 0.0);
                color.g = max(color.g, 0.0);
                color.b = max(color.b, 0.0);

                fragColor = vec4(color, 1.0);

//                // Calculate direct lighting...
//                vec3 directLighting = vec3(0.0);
//                for (int i = 0; i < 1; ++i) {
//                    directLighting += calcDirLight(directionalLights.directionalLight, N, V, fragWorldPos, material, F0);
//                }
//                for (int i = 0; i < pointLights.lightCount; ++i) {
//                    directLighting += calcPointLight(pointLights.lights[i], N, V, fragWorldPos, material, F0);
//                }
//                for (int i = 0; i < spotlights.lightCount; ++i) {
//                    directLighting += calcSpotlight(spotlights.lights[i], N, V, fragWorldPos, material, F0);
//                }
//
//                // Calculate ambient lighting...
//                vec3 ambient = vec3(0.03) * material.albedo * material.ao;
//
//                // Combine ambient and direct lighting...
//                vec3 colorLinear = ambient + directLighting;
//
//                // Convert linear color to gamma space...
//                vec3 colorGamma = pow(colorLinear, vec3(1.0 / 2.2));
//
//                // Output final color...
//                fragColor = vec4(colorGamma, 1.0);

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

    // create compute pipeline
    {
        // a test compute pass to transform the testRenderTextureImage, which is a texture that will be used to render view of a camera
        // we will will make a pixelation effect with this compute shader
        // the input image is read-write

        // reference : "#pragma kernel Pixelate
        //
        //RWTexture2D<float4> _Result;
        //
        //int _BlockSize;
        //int _ResultWidth;
        //int _ResultHeight;
        //
        //[numthreads(8,8,1)]
        //void Pixelate (uint3 id : SV_DispatchThreadID)
        //{
        //    if (id.x >= _ResultWidth || id.y >= _ResultHeight)
        //        return;
        //
        //    const float2 startPos = id.xy * _BlockSize;
        //
        //    if (startPos.x >= _ResultWidth || startPos.y >= _ResultHeight)
        //        return;
        //
        //    const int blockWidth = min(_BlockSize, _ResultWidth - startPos.x);
        //    const int blockHeight = min(_BlockSize, _ResultHeight - startPos.y);
        //    const int numPixels = blockHeight * blockWidth;
        //
        //    float4 colour = float4(0, 0, 0, 0);
        //    for (int i = 0; i < blockWidth; ++i)
        //    {
        //        for (int j = 0; j < blockHeight; ++j)
        //        {
        //            const uint2 pixelPos = uint2(startPos.x + i, startPos.y + j);
        //            colour += _Result[pixelPos];
        //        }
        //    }
        //    colour /= numPixels;
        //
        //    for (int i = 0; i < blockWidth; ++i)
        //    {
        //        for (int j = 0; j < blockHeight; ++j)
        //        {
        //            const uint2 pixelPos = uint2(startPos.x + i, startPos.y + j);
        //            _Result[pixelPos] = colour;
        //        }
        //    }
        //}"
        auto cs = R"(
            #version 450
            layout(local_size_x = 16, local_size_y = 16) in;

            layout(binding = 0, rgba8) uniform image2D rwImage;

            layout(binding = 0) buffer Settings {
                int blockSize;
                int resultWidth;
                int resultHeight;
            };

            void main() {
                ivec2 id = ivec2(gl_GlobalInvocationID.xy);
                if (id.x >= resultWidth || id.y >= resultHeight)
                    return;

                ivec2 startPos = id.xy * blockSize;

                if (startPos.x >= resultWidth || startPos.y >= resultHeight)
                    return;

                int blockWidth = min(blockSize, resultWidth - startPos.x);
                int blockHeight = min(blockSize, resultHeight - startPos.y);
                int numPixels = blockHeight * blockWidth;

                vec4 colour = vec4(0.0, 0.0, 0.0, 0.0);
                for (int i = 0; i < blockWidth; ++i)
                {
                    for (int j = 0; j < blockHeight; ++j)
                    {
                        ivec2 pixelPos = ivec2(startPos.x + i, startPos.y + j);
                        colour += imageLoad(rwImage, pixelPos);
                    }
                }
                colour /= float(numPixels);

                for (int i = 0; i < blockWidth; ++i)
                {
                    for (int j = 0; j < blockHeight; ++j)
                    {
                        ivec2 pixelPos = ivec2(startPos.x + i, startPos.y + j);
                        imageStore(rwImage, pixelPos, colour);
                    }
                }
            }
        )";

        auto computeShader = renderer.getDevice().createShaderModule({
            .type = ShaderModuleType::Compute,
            .code = cs
        });

        auto computeShaderStage = renderer.getDevice().createPipelineShaderStages(PipelineShaderStagesDesc::fromComputeModule(computeShader));

        computePipeline = renderer.getDevice().createComputePipeline({
                .shaderStages = computeShaderStage,
                .imagesMap = {
                        {0, "rwImage"}
                },
                .buffersMap = {
                        {0, "Settings"}
                }
        });



        computeSettingsBuffer = renderer.getDevice().createBuffer(BufferDesc{
                .type = BufferDesc::BufferTypeBits::Storage,
                .data = nullptr,
                .size = sizeof(int) * 3,
                .storage = ResourceStorage::Shared
        });
    }

    // bloom compute pipelines
    {
        // downscale
        {
            auto cs = R"(
            #version 460

            layout(binding = 0)			 uniform sampler2D u_input_texture;
            layout(rgba32f, binding = 0) uniform writeonly image2D u_output_image;

            // settings uniform block
            layout(binding = 0) buffer Settings {
                vec4 u_threshold;
                vec2 u_texel_size;
                int u_mip_level;
                int u_use_threshold;
            };

//            uniform vec4  u_threshold; // x -> threshold, yzw -> (threshold - knee, 2.0 * knee, 0.25 * knee)
//            uniform vec2  u_texel_size;
//            uniform int   u_mip_level;
//            uniform bool  u_use_threshold;

            const float epsilon = 1.0e-4;

            // Curve = (threshold - knee, knee * 2.0, knee * 0.25)
            vec4 quadratic_threshold(vec4 color, float threshold, vec3 curve)
            {
                // Pixel brightness
                float br = max(color.r, max(color.g, color.b));

                // Under-threshold part: quadratic curve
                float rq = clamp(br - curve.x, 0.0, curve.y);
                rq = curve.z * rq * rq;

                // Combine and apply the brightness response curve.
                color *= max(rq, br - threshold) / max(br, epsilon);

                return color;
            }

            float luma(vec3 c)
            {
                return dot(c, vec3(0.2126729, 0.7151522, 0.0721750));
            }

            // [Karis2013] proposed reducing the dynamic range before averaging
            vec4 karis_avg(vec4 c)
            {
                return c / (1.0 + luma(c.rgb));
            }

            #define GROUP_SIZE         8
            #define GROUP_THREAD_COUNT (GROUP_SIZE * GROUP_SIZE)
            #define FILTER_SIZE        3
            #define FILTER_RADIUS      (FILTER_SIZE / 2)
            #define TILE_SIZE          (GROUP_SIZE + 2 * FILTER_RADIUS)
            #define TILE_PIXEL_COUNT   (TILE_SIZE * TILE_SIZE)

            shared float sm_r[TILE_PIXEL_COUNT];
            shared float sm_g[TILE_PIXEL_COUNT];
            shared float sm_b[TILE_PIXEL_COUNT];

            void store_lds(int idx, vec4 c)
            {
                sm_r[idx] = c.r;
                sm_g[idx] = c.g;
                sm_b[idx] = c.b;
            }

            vec4 load_lds(uint idx)
            {
                return vec4(sm_r[idx], sm_g[idx], sm_b[idx], 1.0);
            }

            layout(local_size_x = GROUP_SIZE, local_size_y = GROUP_SIZE) in;
            void main()
            {
                ivec2 pixel_coords = ivec2(gl_GlobalInvocationID);
                ivec2 base_index   = ivec2(gl_WorkGroupID) * GROUP_SIZE - FILTER_RADIUS;

                // The first (TILE_PIXEL_COUNT - GROUP_THREAD_COUNT) threads load at most 2 texel values
                for (int i = int(gl_LocalInvocationIndex); i < TILE_PIXEL_COUNT; i += GROUP_THREAD_COUNT)
                {
                    vec2 uv        = (vec2(base_index) + 0.5) * u_texel_size;
                    vec2 uv_offset = vec2(i % TILE_SIZE, i / TILE_SIZE) * u_texel_size;

                    vec4 color = textureLod(u_input_texture, uv + uv_offset, u_mip_level);
                    store_lds(i, color);
                }

                memoryBarrierShared();
                barrier();

                // Based on [Jimenez14] http://goo.gl/eomGso
                // center texel
                uint sm_idx = (gl_LocalInvocationID.x + FILTER_RADIUS) + (gl_LocalInvocationID.y + FILTER_RADIUS) * TILE_SIZE;

                vec4 A = load_lds(sm_idx - TILE_SIZE - 1);
                vec4 B = load_lds(sm_idx - TILE_SIZE    );
                vec4 C = load_lds(sm_idx - TILE_SIZE + 1);
                vec4 F = load_lds(sm_idx - 1            );
                vec4 G = load_lds(sm_idx                );
                vec4 H = load_lds(sm_idx + 1            );
                vec4 K = load_lds(sm_idx + TILE_SIZE - 1);
                vec4 L = load_lds(sm_idx + TILE_SIZE    );
                vec4 M = load_lds(sm_idx + TILE_SIZE + 1);

                vec4 D = (A + B + G + F) * 0.25;
                vec4 E = (B + C + H + G) * 0.25;
                vec4 I = (F + G + L + K) * 0.25;
                vec4 J = (G + H + M + L) * 0.25;

                vec2 div = (1.0 / 4.0) * vec2(0.5, 0.125);

                vec4 c =  karis_avg((D + E + I + J) * div.x);
                     c += karis_avg((A + B + G + F) * div.y);
                     c += karis_avg((B + C + H + G) * div.y);
                     c += karis_avg((F + G + L + K) * div.y);
                     c += karis_avg((G + H + M + L) * div.y);

                if (u_use_threshold != 0)
                {
                    c = quadratic_threshold(c, u_threshold.x, u_threshold.yzw);
                }

                imageStore(u_output_image, pixel_coords, c);
            }
        )";

            auto computeShader = renderer.getDevice().createShaderModule({.type = ShaderModuleType::Compute,
                                                                          .code = cs});

            auto computeShaderStage = renderer.getDevice().createPipelineShaderStages(PipelineShaderStagesDesc::fromComputeModule(computeShader));

            bloomDownscalePipeline = renderer.getDevice().createComputePipeline({
                    .shaderStages = computeShaderStage,
                    .imagesMap = {
                            {0, "u_output_image"}
                    },
                    .texturesMap = {{0, "u_input_texture"}},
                    .buffersMap = {
                            {0, "Settings"}
                    }
            });

            BloomDownscaleSettings dummySettings{};
            bloomDownscaleSettingsBuffer = renderer.getDevice().createBuffer(BufferDesc{
                    .type = BufferDesc::BufferTypeBits::Storage,
                    .data = &dummySettings,
                    .size = sizeof(dummySettings),
                    .storage = ResourceStorage::Shared
            });
        }

        // upscale
        {
            auto cs = R"(
                #version 460

                layout(binding = 0)			 uniform sampler2D u_input_texture;
                layout(rgba32f, binding = 0) uniform image2D   u_output_image;

                layout(binding = 1)			 uniform sampler2D u_dirt_texture;

                layout(binding = 0) buffer Settings {
                    vec2  u_texel_size;
                    int   u_mip_level;
                    float u_bloom_intensity;
                    float u_dirt_intensity;
                };

//                uniform vec2  u_texel_size;
//                uniform int   u_mip_level;
//                uniform float u_bloom_intensity;
//                uniform float u_dirt_intensity;

                #define GROUP_SIZE         8
                #define GROUP_THREAD_COUNT (GROUP_SIZE * GROUP_SIZE)
                #define FILTER_SIZE        3
                #define FILTER_RADIUS      (FILTER_SIZE / 2)
                #define TILE_SIZE          (GROUP_SIZE + 2 * FILTER_RADIUS)
                #define TILE_PIXEL_COUNT   (TILE_SIZE * TILE_SIZE)

                shared float sm_r[TILE_PIXEL_COUNT];
                shared float sm_g[TILE_PIXEL_COUNT];
                shared float sm_b[TILE_PIXEL_COUNT];

                void store_lds(int idx, vec4 c)
                {
                    sm_r[idx] = c.r;
                    sm_g[idx] = c.g;
                    sm_b[idx] = c.b;
                }

                vec4 load_lds(uint idx)
                {
                    return vec4(sm_r[idx], sm_g[idx], sm_b[idx], 1.0);
                }

                layout(local_size_x = GROUP_SIZE, local_size_y = GROUP_SIZE) in;
                void main()
                {
                    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
                    vec2  base_index   = ivec2(gl_WorkGroupID) * GROUP_SIZE - FILTER_RADIUS;

                    // The first (TILE_PIXEL_COUNT - GROUP_THREAD_COUNT) threads load at most 2 texel values
                    for (int i = int(gl_LocalInvocationIndex); i < TILE_PIXEL_COUNT; i += GROUP_THREAD_COUNT)
                    {
                        vec2 uv        = (base_index + 0.5) * u_texel_size;
                        vec2 uv_offset = vec2(i % TILE_SIZE, i / TILE_SIZE) * u_texel_size;

                        vec4 color = textureLod(u_input_texture, (uv + uv_offset), u_mip_level);
                        store_lds(i, color);
                    }

                    memoryBarrierShared();
                    barrier();

                    // center texel
                    uint sm_idx = (gl_LocalInvocationID.x + FILTER_RADIUS) + (gl_LocalInvocationID.y + FILTER_RADIUS) * TILE_SIZE;

                    // Based on [Jimenez14] http://goo.gl/eomGso
                    vec4 s;
                    s =  load_lds(sm_idx - TILE_SIZE - 1);
                    s += load_lds(sm_idx - TILE_SIZE    ) * 2.0;
                    s += load_lds(sm_idx - TILE_SIZE + 1);

                    s += load_lds(sm_idx - 1) * 2.0;
                    s += load_lds(sm_idx    ) * 4.0;
                    s += load_lds(sm_idx + 1) * 2.0;

                    s += load_lds(sm_idx + TILE_SIZE - 1);
                    s += load_lds(sm_idx + TILE_SIZE    ) * 2.0;
                    s += load_lds(sm_idx + TILE_SIZE + 1);

                    vec4 bloom = s * (1.0 / 16.0);

                    vec4 out_pixel = imageLoad(u_output_image, pixel_coords);
                         out_pixel += bloom * u_bloom_intensity;

                    if (u_mip_level == 1)
                    {
                        vec2  uv  = (vec2(pixel_coords) + vec2(0.5, 0.5)) * u_texel_size;
                        out_pixel += texture(u_dirt_texture, uv) * u_dirt_intensity * bloom * u_bloom_intensity;
                    }

                    imageStore(u_output_image, pixel_coords, out_pixel);
                }
            )";

            auto computeShader = renderer.getDevice().createShaderModule({.type = ShaderModuleType::Compute,
                                                                          .code = cs});

            auto computeShaderStage = renderer.getDevice().createPipelineShaderStages(PipelineShaderStagesDesc::fromComputeModule(computeShader));

            bloomUpscalePipeline = renderer.getDevice().createComputePipeline({
                    .shaderStages = computeShaderStage,
                    .imagesMap = {
                            {0, "u_output_image"}
                    },
                    .texturesMap = {{0, "u_input_texture"}, {1, "u_dirt_texture"}},
                    .buffersMap = {
                            {0, "Settings"}
                    }
            });

            BloomUpscaleSettings dummySettings{};
            bloomUpscaleSettingsBuffer = renderer.getDevice().createBuffer(BufferDesc{
                    .type = BufferDesc::BufferTypeBits::Storage,
                    .data = &dummySettings,
                    .size = sizeof(dummySettings),
                    .storage = ResourceStorage::Shared
            });
        }
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
        Light spotlight;
        spotlight.setSpot(12.5, 17.5);
//        spotlight.setDirectional();
        spotlight.setColor({1.0f, 1.0f, 1.0f});
        spotlight.setIntensity(2.0f);
        EntityView viewer = defaultScene->createEntity("viewer");
        viewer.addComponent<CameraComponent>(activeCamera);
        viewer.addComponent<LightComponent>(spotlight);
        viewer.getSceneNode().getTransform().setPosition({0.0f, 6.0f, 3.0f});
        viewer.getSceneNode().getTransform().setRotation({glm::radians(-20.0f), 0, 0.0f});

        EntityView cow = defaultScene->createEntity("cow");
        cow.addComponent<MeshComponent>(resourceManager.getMeshByName("spider"), resourceManager.getMaterialByName("pbrDefaultMaterial"));
        cow.getSceneNode().getTransform().setPosition({-7.0f, 0.0f, -4.0f});
        cow.getSceneNode().getTransform().setScale(glm::vec3(1.0f));
        cow.getSceneNode().getTransform().setRotation({0.0f, glm::radians(20.0f), 0.0f});

        EntityView bunny = defaultScene->createEntity("bunny");
        bunny.addComponent<MeshComponent>(resourceManager.getMeshByName("bunny"), resourceManager.getMaterialByName("pbrDefaultMaterial"));
        bunny.getSceneNode().getTransform().setPosition({0.0f, 0.0f, 0.0f});
        bunny.getSceneNode().getTransform().setScale(glm::vec3(2.0f));
        bunny.getSceneNode().getTransform().setRotation({0.0f, 3.0f, 0.0f});

        EntityView root = defaultScene->createEntity("teapot");
        root.addComponent<MeshComponent>(resourceManager.getMeshByName("teapot"), resourceManager.getMaterialByName("pbrDefaultMaterial"));

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
            testRenderTexture = renderer.getDevice().createTexture(TextureDesc::new2D(TextureFormat::RGBA_UNorm8, desc.width, desc.height, TextureDesc::TextureUsageBits::Attachment | TextureDesc::TextureUsageBits::Sampled | TextureDesc::TextureUsageBits::Storage));
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


//        // Create a child node
//        EntityView spherePortal = defaultScene->createEntity("spherePortal");
//
//        Light dirLight;
//        dirLight.setDirectional();
//        spherePortal.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("pbrGlowMaterial"));
////        spherePortal.addComponent<LightComponent>(dirLight);
//        auto& spherePortalNode = spherePortal.getSceneNode();
//        spherePortalNode.getTransform().setPosition({10.0f, 3.0f, 10.0f});
//        spherePortalNode.getTransform().setScale({1.f, 1.f, 1.f});
//        spherePortalNode.getTransform().setRotation({0.0f, 0.0f, 0.0f});

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

    // create a room with a floor and walls
    {
        auto room = defaultScene->createEntity("room");

        float width = 20.0f;
        float height = 10.0f;
        float depth = 20.0f;
        float thickness = 1.0f;
        float margin = 0.1f;
        // floor
        {
            auto floor = defaultScene->createEntity("floor");
            floor.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("pbrMaterial2"));
            {
                auto& floorNode = floor.getSceneNode();
                floorNode.getTransform().setPosition({0.0f, -height - thickness, 0.0f});
                floorNode.getTransform().setScale({width, thickness, depth});
                floorNode.getTransform().setRotation({0.0f, 0.0f, 0.0f});

                room.getSceneNode().addChild(&floorNode);
            }
        }

        // ceiling
        {
            auto ceiling = defaultScene->createEntity("ceiling");
            ceiling.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("pbrMaterial2"));
            {
                auto& ceilingNode = ceiling.getSceneNode();
                ceilingNode.getTransform().setPosition({0.0f, height + thickness, 0.0f});
                ceilingNode.getTransform().setScale({width, thickness, depth});
                ceilingNode.getTransform().setRotation({0.0f, 0.0f, 0.0f});

                room.getSceneNode().addChild(&ceilingNode);
            }
        }

        // left wall
        {
            auto leftWall = defaultScene->createEntity("leftWall");
            leftWall.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("pbrMaterial2"));
            {
                auto& leftWallNode = leftWall.getSceneNode();
                leftWallNode.getTransform().setPosition({-width - thickness - margin, 0.0f, 0.0f});
                leftWallNode.getTransform().setScale({thickness, height + 2*thickness, depth});
                leftWallNode.getTransform().setRotation({0.0f, 0.0f, 0.0f});

                room.getSceneNode().addChild(&leftWallNode);
            }
        }

        // right wall
        {
            auto rightWall = defaultScene->createEntity("rightWall");
            rightWall.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("pbrMaterial2"));
            {
                auto& rightWallNode = rightWall.getSceneNode();
                rightWallNode.getTransform().setPosition({width + thickness + margin, 0.0f, 0.0f});
                rightWallNode.getTransform().setScale({thickness, height + 2*thickness, depth});
                rightWallNode.getTransform().setRotation({0.0f, 0.0f, 0.0f});

                room.getSceneNode().addChild(&rightWallNode);
            }
        }

        // back wall
        {
            auto backWall = defaultScene->createEntity("backWall");
            backWall.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("pbrMaterial2"));
            {
                auto& backWallNode = backWall.getSceneNode();
                backWallNode.getTransform().setPosition({0.0f, 0.0f, -depth - thickness - margin});
                backWallNode.getTransform().setScale({width + 2*thickness, height + 2*thickness, thickness});
                backWallNode.getTransform().setRotation({0.0f, 0.0f, 0.0f});

                room.getSceneNode().addChild(&backWallNode);
            }
        }

        //red light
        {
            // Create a child node
            EntityView lightstick = defaultScene->createEntity("red_lightstick");

            Light light;
            light.setPoint();
            light.setIntensity(10.0f);
            light.setColor({1.0f, 0.0f, 0.0f});
            lightstick.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("pbrGlowMaterial"));
            lightstick.addComponent<LightComponent>(light);
            auto& lightstickNode = lightstick.getSceneNode();
            lightstickNode.getTransform().setPosition({2.4f, -4.5f, -2.3f});
            lightstickNode.getTransform().setScale({1.f, 1.f, 27});
            lightstickNode.getTransform().setRotation({glm::radians(45.0f), glm::radians(21.0f), glm::radians(30.0f)});
        }

        // blue light
        {
            // Create a child node
            EntityView lightstick = defaultScene->createEntity("blue_lightstick");

            Light light;
            light.setPoint();
            light.setIntensity(10.0f);
            light.setColor({0.0f, 0.0f, 1.0f});
            lightstick.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("pbrGlowMaterialBlue"));
            lightstick.addComponent<LightComponent>(light);
            auto& lightstickNode = lightstick.getSceneNode();
            lightstickNode.getTransform().setPosition({-12.4f, -1.8f, 4.8f});
            lightstickNode.getTransform().setScale({1.f, 1.f, 24.5f});
            lightstickNode.getTransform().setRotation({glm::radians(96.0f), glm::radians(-89.0f), glm::radians(-30.0f)});
        }

//        //green light
//        {
//            // Create a child node
//            EntityView lightstick = defaultScene->createEntity("green_lightstick");
//
//            Light light;
//            light.setPoint();
//            light.setIntensity(10.0f);
//            light.setColor({0.0f, 1.0f, 0.0f});
//            lightstick.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("pbrGlowMaterialGreen"));
//            lightstick.addComponent<LightComponent>(light);
//            auto& lightstickNode = lightstick.getSceneNode();
//            lightstickNode.getTransform().setPosition({0, 0, 0});
//            lightstickNode.getTransform().setScale({1.f, 1.f, 1.f});
//            lightstickNode.getTransform().setRotation({0.f, 0.f, 0.f});
//        }

        float openingWidth = 8.0f;
        // create a front wall with an opening of width openingWidth
        // composed of two walls

        // left side
        {
            auto frontWallLeft = defaultScene->createEntity("frontWallLeft");
            frontWallLeft.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("pbrMaterial2"));
            {
                auto& frontWallLeftNode = frontWallLeft.getSceneNode();
                frontWallLeftNode.getTransform().setPosition({-width/2 - openingWidth/2 - margin - thickness, 0.0f, depth + thickness + margin + 15});
                frontWallLeftNode.getTransform().setScale({width/2 - openingWidth/2 + thickness, height + 2*thickness, thickness});
                frontWallLeftNode.getTransform().setRotation({0.0f, 0.0f, 0.0f});

                room.getSceneNode().addChild(&frontWallLeftNode);
            }
        }

        // right side
        {
            auto frontWallRight = defaultScene->createEntity("frontWallRight");
            frontWallRight.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("pbrMaterial2"));
            {
                auto& frontWallRightNode = frontWallRight.getSceneNode();
                frontWallRightNode.getTransform().setPosition({width/2 + openingWidth/2 + margin + thickness, 0.0f, depth + thickness + margin});
                frontWallRightNode.getTransform().setScale({width/2 - openingWidth/2 + thickness, height + 2*thickness, thickness});
                frontWallRightNode.getTransform().setRotation({0.0f, 0.0f, 0.0f});

                room.getSceneNode().addChild(&frontWallRightNode);
            }
        }

        // black object
        {
            auto blackObject = defaultScene->createEntity("blackObject");

            blackObject.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("pbrBlack"));
            {
                auto& node = blackObject.getSceneNode();
                node.getTransform().setPosition({0.0f, -5.0f, 15.4f});
                node.getTransform().setScale({3.0f, 5.0f, 0.5f});
                node.getTransform().setRotation({0.0f, 0.0f, 0.0f});
            }
        }

        float shadowTestWallDistance = 15.0f;
        // shadow test wall
        {
            auto shadowTestWall = defaultScene->createEntity("shadowTestWall");

            shadowTestWall.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("pbrMaterial2"));
            {
                auto& node = shadowTestWall.getSceneNode();
                node.getTransform().setPosition({0, 0.0f, depth + shadowTestWallDistance*2 + thickness*2 + margin*2});
                node.getTransform().setScale({width + 2*thickness, height + 2*thickness, thickness});
                node.getTransform().setRotation({0.0f, 0.0f, 0.0f});

                room.getSceneNode().addChild(&node);
            }
        }

        // floor connecting the room and the shadowTestWall
        {
            auto floor = defaultScene->createEntity("floor2");
            floor.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("pbrMaterial2"));
            {
                auto& floorNode = floor.getSceneNode();
                floorNode.getTransform().setPosition({0.0f, -height - thickness, depth + shadowTestWallDistance + thickness*2});
                floorNode.getTransform().setScale({width, thickness, shadowTestWallDistance});
                floorNode.getTransform().setRotation({0.0f, 0.0f, 0.0f});

                room.getSceneNode().addChild(&floorNode);
            }
        }
    }


    // portal and its frame
    {
        auto portal = defaultScene->createEntity("portal");
//        portal.addComponent<MeshComponent>(resourceManager.getMeshByName("portal"), resourceManager.getMaterialByName("portalMaterial"));
//        {
//            auto& portalNode = portal.getSceneNode();
//            portalNode.getTransform().setPosition({0.0f, 0.0f, 0.0f});
//            portalNode.getTransform().setScale({5.0f, 5.0f, 1.0f});
//            portalNode.getTransform().setRotation({glm::radians(90.0f), 0.0f, 0.0f});
//
//            auto& portalMaterialComponent = portal.getComponent<MeshComponent>();
//            auto portalMaterial_ = portalMaterialComponent.getMaterial()->getMaterial();
//            auto samplerState = renderer.getDevice().createSamplerState(SamplerStateDesc::newLinear());
//            portalMaterial_->setTextureSampler("tex", testRenderTexture, samplerState, 0);
//        }

        // create a rectangular frame around the portal made of 4 cubes stretched to be a frame
        auto portalFrame = defaultScene->createEntity("portalFrame");

//        // transform the frame pieces according to i
//        for (int i = 0; i < 4; i++)
//        {
//            auto framePart = defaultScene->createEntity("framePart" + std::to_string(i));
//            framePart.addComponent<MeshComponent>(resourceManager.getMeshByName("portalFrame"), resourceManager.getMaterialByName("normalMaterial"));
//            auto& framePartNode = framePart.getSceneNode();
//
//            // transform the frame part according to i, add a scale parameter for the thickness, and the transformations are TRS
//            switch (i)
//            {
//                case 0:
//                    framePartNode.getTransform().setPosition({0.0f, 0.0f, -5.0f});
//                    framePartNode.getTransform().setScale({5.0f, 0.1f, 0.1f});
//                    break;
//                case 1:
//                    framePartNode.getTransform().setPosition({0.0f, 0.0f, 5.0f});
//                    framePartNode.getTransform().setScale({5.0f, 0.1f, 0.1f});
//                    break;
//                case 2:
//                    framePartNode.getTransform().setPosition({-5.0f, 0.0f, 0.0f});
//                    framePartNode.getTransform().setScale({0.1f, 0.1f, 5.0f});
//                    break;
//                case 3:
//                    framePartNode.getTransform().setPosition({5.0f, 0.0f, 0.0f});
//                    framePartNode.getTransform().setScale({0.1f, 0.1f, 5.0f});
//                    break;
//            }
//
//            portalFrame.getSceneNode().addChild(&framePartNode);
//            portalFrame.getSceneNode().getTransform().setRotation({glm::radians(90.0f), 0.0f, 0.0f});
//        }
//        portalFrame.getSceneNode().addChild(&portal.getSceneNode());
//        portalFrame.getSceneNode().getTransform().setPosition({20.0f, 0.0f, 0.0f});
////        portal.getSceneNode().addChild(&portalFrame.getSceneNode());
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
            root.visit([](SceneNode& node) {
                node.setVisible(false);
            });
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


        if (input.isMouseDragging(1))
        {
//            std::cout << "Mouse dragging" << std::endl;
//            std::cout << "Mouse drag delta: " << input.getMouseDragDeltaX() << ", " << input.getMouseDragDeltaY() << std::endl;
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
            viewerNode.getTransform().translate(viewerNode.getTransform().getForward() * speed);
        }
        if (input.isKeyPressed(input::KeyCode::S))
        {
            // move backward relative to orientation
            viewerNode.getTransform().translate(viewerNode.getTransform().getForward() * -speed);
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
        if (input.isKeyPressed(input::KeyCode::Space))
        {
            // move up
            viewerNode.getTransform().translate(glm::vec3(0.0, 1.0, 0.0) * speed);
        }
        if (input.isKeyPressed(input::KeyCode::LeftControl))
        {
            // move down
            viewerNode.getTransform().translate(glm::vec3(0.0, 1.0, 0.0) * -speed);
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
                auto pbrMaterial = resourceManager.getMaterialByName("pbrDefaultMaterial")->getMaterial();

                constexpr int MAX_LIGHTS = 10;

                struct LightUniform {
                    alignas(16) glm::vec3 position;
                    alignas(16) glm::vec3 color;
                    float intensity;
                    float constant;
                    float linear;
                    float quadratic;
                };

                struct SpotlightUniform {
                    alignas(16) glm::vec3 position;
                    alignas(16) glm::vec3 direction;
                    alignas(16) glm::vec3 color;

                    float cutoff;
                    float outerCutoff;

                    float intensity;
                    float constant;
                    float linear;
                    float quadratic;
                };

                struct LightStruct
                {
                    alignas(16) glm::vec3 position;
                    alignas(16) glm::vec3 color;

                };
                struct PointLightsUBO
                {
                    int numLights;
                    LightUniform lights[MAX_LIGHTS];
                };

                struct SpotlightsUBO
                {
                    int numLights;
                    SpotlightUniform lights[MAX_LIGHTS];
                };

                struct DirecitonalLightUniform
                {
                    alignas(16) glm::vec3 color;
                    alignas(16) glm::vec3 direction;
                    float intensity;
                };

                struct DirectionalLightsUBO
                {
                    DirecitonalLightUniform light;
                };

                PointLightsUBO pointLightsUbo{};
                SpotlightsUBO spotlightsUbo{};
                DirectionalLightsUBO directionalLightsUbo{};


                for (int i = 0; i < sceneRenderData.lights.size(); i++)
                {
                    auto item = sceneRenderData.lights[i];
                    switch (item.light->getType())
                    {
                        case LightType::Point:
                        {
                            auto& pointLight = item.light;
                            pointLightsUbo.lights[pointLightsUbo.numLights].position = item.position;
                            pointLightsUbo.lights[pointLightsUbo.numLights].color = pointLight->getColor();
                            pointLightsUbo.lights[pointLightsUbo.numLights].intensity = pointLight->getIntensity();
                            pointLightsUbo.lights[pointLightsUbo.numLights].constant = pointLight->getConstant();
                            pointLightsUbo.lights[pointLightsUbo.numLights].linear = pointLight->getLinear();
                            pointLightsUbo.lights[pointLightsUbo.numLights].quadratic = pointLight->getQuadratic();
                            pointLightsUbo.numLights++;
                            break;
                        }
                        case LightType::Spot:
                        {
                            auto& spotlight = item.light;
                            spotlightsUbo.lights[spotlightsUbo.numLights].position = item.position;
                            spotlightsUbo.lights[spotlightsUbo.numLights].direction = item.direction;
                            spotlightsUbo.lights[spotlightsUbo.numLights].color = spotlight->getColor();
                            spotlightsUbo.lights[spotlightsUbo.numLights].cutoff = glm::cos(glm::radians(spotlight->getCutOff()));
                            spotlightsUbo.lights[spotlightsUbo.numLights].outerCutoff = glm::cos(glm::radians(spotlight->getOuterCutOff()));
                            spotlightsUbo.lights[spotlightsUbo.numLights].intensity = spotlight->getIntensity();
                            spotlightsUbo.lights[spotlightsUbo.numLights].constant = spotlight->getConstant();
                            spotlightsUbo.lights[spotlightsUbo.numLights].linear = spotlight->getLinear();
                            spotlightsUbo.lights[spotlightsUbo.numLights].quadratic = spotlight->getQuadratic();
                            spotlightsUbo.numLights++;
                            break;
                        }
                        case LightType::Directional:
                        {
                            auto& directionalLight = item.light;
                            directionalLightsUbo.light.color = directionalLight->getColor();
                            directionalLightsUbo.light.direction = item.direction;
                            directionalLightsUbo.light.intensity = directionalLight->getIntensity();
                            break;
                        }
                    }
                }

                pbrMaterial->setUniformBytes("Lights", &pointLightsUbo, sizeof(pointLightsUbo), 6); // binding 6
                pbrMaterial->setUniformBytes("Spotlights", &spotlightsUbo, sizeof(spotlightsUbo), 7); // binding 7
                pbrMaterial->setUniformBytes("DirectionalLight", &directionalLightsUbo, sizeof(directionalLightsUbo), 8); // binding 8
            }

            renderer.begin(camera.getRenderTarget());
            sceneRenderer.render(renderer, sceneRenderData, {cameraNode->getWorldTransform().getPosition(),
                                                             cameraNode->getWorldTransform().getForward(),
                                                             glm::inverse(cameraNode->getWorldTransform().getModel()), camera.getCamera()->getProjection(), camera.getCamera()->getViewportWidth(), camera.getCamera()->getViewportHeight()});
            renderer.end();
        }


        {
            Settings computeSettings = {
                    .blockSize = 32,
                    .resultWidth = static_cast<int>(testRenderTexture->getWidth()),
                    .resultHeight = static_cast<int>(testRenderTexture->getHeight())
            };

            computeSettingsBuffer->data(&computeSettings, sizeof(computeSettings), 0);

            auto cmdPool = renderer.getDevice().createCommandPool({});

            auto cmdBuffer = cmdPool->acquireComputeCommandBuffer({});
            cmdBuffer->begin();

            cmdBuffer->bindComputePipeline(computePipeline);
            cmdBuffer->bindImage(0, testRenderTexture, ReadWrite);
            cmdBuffer->bindBuffer(0, computeSettingsBuffer, 0);
            cmdBuffer->dispatch({static_cast<uint32_t>(testRenderTexture->getWidth() / 16), static_cast<uint32_t>(testRenderTexture->getHeight() / 16), 1});

            cmdBuffer->end();
            cmdPool->submitCommandBuffer(std::move(cmdBuffer));
        }

        if (mainCamera)
        {
            auto& camera = mainCamera->getEntityView().getComponent<CameraComponent>();
            SceneRenderData sceneRenderData;
            activeScene->getSceneRenderData(sceneRenderData);
            renderer.begin(RenderPassBeginDesc{
                    .renderPass = {
                            .colorAttachments = {
                                    RenderPassDesc::ColorAttachmentDesc{
                                            LoadAction::Clear,
                                            StoreAction::Store,
                                            {0.0f, 0.0f, 0.0f, 1.0f}
                                    }
                            },
                            .depthAttachment = RenderPassDesc::DepthAttachmentDesc{
                                    LoadAction::Clear,
                                    StoreAction::Store,
                                    0, 0,
                                    1.0f
                            }
                    },
                    .framebuffer = hdrFramebuffer
            });
            renderer.bindViewport({0,0, static_cast<float>(desc.width), static_cast<float>(desc.height)});
            sceneRenderer.render(renderer, sceneRenderData, {mainCamera->getWorldTransform().getPosition(),
                                                             mainCamera->getWorldTransform().getForward(),
                                                             glm::inverse(mainCamera->getWorldTransform().getModel()), camera.getCamera()->getProjection(), camera.getCamera()->getViewportWidth(), camera.getCamera()->getViewportHeight()});
            renderer.end();

            //raytracing


            auto cmdPool = renderer.getDevice().createCommandPool({});

            auto cmdBuffer = cmdPool->acquireComputeCommandBuffer({});
            // bloom filter downscale
            {
                hdrColorBuffers[0]->generateMipmap();
                cmdBuffer->begin();
                cmdBuffer->bindComputePipeline(bloomDownscalePipeline);

                cmdBuffer->bindTexture(0, hdrColorBuffers[0]);
                cmdBuffer->bindSamplerState(0, hdrSampler);

                BloomDownscaleSettings settings;
                settings.threshold = glm::vec4(bloomThreshold, bloomThreshold - bloomKnee, 2.0f * bloomKnee, 0.25f * bloomKnee);

                glm::uvec2 mipSize = {hdrColorBuffers[0]->getWidth()/2, hdrColorBuffers[0]->getHeight()/2};
                for (uint8_t i = 0; i < hdrColorBuffers[0]->getNumMipLevels() - 1; ++i)
                {
                    settings.texelSize = 1.0f / glm::vec2(mipSize);
                    settings.mipLevel = i;
                    settings.useThreshold = i == 0;
                    bloomDownscaleSettingsBuffer->data(&settings, sizeof(settings), 0);

                    cmdBuffer->bindImage(0, hdrColorBuffers[0], WriteOnly, i + 1);
                    cmdBuffer->bindBuffer(0, bloomDownscaleSettingsBuffer, 0);
                    cmdBuffer->dispatch({static_cast<uint32_t>(glm::ceil(float(mipSize.x) / 8)), static_cast<uint32_t>(glm::ceil(float(mipSize.y) / 8)), 1});

                    mipSize = mipSize/2u;
                }
            }

            // bloom filter upscale
            {
                cmdBuffer->bindComputePipeline(bloomUpscalePipeline);

                BloomUpscaleSettings settings;
                settings.bloomIntensity = bloomIntensity;
                cmdBuffer->bindTexture(0, hdrColorBuffers[0]);
                cmdBuffer->bindSamplerState(0, hdrSampler);

                glm::uvec2 mipSize;
                for (uint8_t i = hdrColorBuffers[0]->getNumMipLevels() - 1; i >= 1; --i)
                {
                    mipSize.x = glm::max(1.0, glm::floor(float(hdrColorBuffers[0]->getWidth())  / glm::pow(2.0, i - 1)));
                    mipSize.y = glm::max(1.0, glm::floor(float(hdrColorBuffers[0]->getHeight()) / glm::pow(2.0, i - 1)));
                    settings.texelSize = 1.0f / glm::vec2(mipSize);
                    settings.mipLevel = i;
//                    settings.dirtIntensity = 0.0f;

                    bloomUpscaleSettingsBuffer->data(&settings, sizeof(settings), 0);

                    cmdBuffer->bindImage(0, hdrColorBuffers[0], ReadWrite, i - 1);
                    cmdBuffer->bindBuffer(0, bloomUpscaleSettingsBuffer, 0);
                    cmdBuffer->dispatch({static_cast<uint32_t>(glm::ceil(float(mipSize.x) / 8)), static_cast<uint32_t>(glm::ceil(float(mipSize.y) / 8)), 1});
                }

                cmdBuffer->end();
            }

            cmdPool->submitCommandBuffer(std::move(cmdBuffer));

            // render hdrColorBuffers[0] to screen quad
            {
                auto mat = resourceManager.getMaterialByName("screenQuadMaterial");

                mat->setUniformBuffer("Settings", &postProcessSettings, sizeof(postProcessSettings), 0);

                mat->use(renderer);
                mat->setTextureSampler("screenTexture", resourceManager.getTextureByName("hdrColorBuffer0"), 0);

                graphics::Renderable screenQuad(mat->getMaterial(), screenQuadBuffer);

                renderer.begin();
                renderer.draw(screenQuad);
                renderer.end();
            }
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
