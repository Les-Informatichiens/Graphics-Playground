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
#include "TempResourceInitializer.h"

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
                bool u_useFXAA;
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

            vec4 filteredSample(sampler2D tex, vec2 uv)
            {
                vec4 x = u_exposure * texture(tex, uv);

                vec3 color = ACESInputMat * x.rgb;
                color = RRTAndODTFit(color);
                color = ACESOutputMat * color;

                color = gammaCorrect(color);
                color = clamp(color, 0.0, 1.0);

                return vec4(color, 1.0);
            }

            vec4 filteredSampleOffset(sampler2D tex, vec2 uv, ivec2 pixelOffset)
            {
                vec4 x = u_exposure * textureOffset(tex, uv, pixelOffset);

                vec3 color = ACESInputMat * x.rgb;
                color = RRTAndODTFit(color);
                color = ACESOutputMat * color;

                color = gammaCorrect(color);
                color = clamp(color, 0.0, 1.0);

                return vec4(color, 1.0);
            }

            float rgb2luma(vec3 rgb){
                return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
            }

            // Iterating
            float QUALITY(int i){
                if(i < 5){
                    return 1.0;
                } else if(i == 5){
                    return 1.5;
                } else if(i == 6){
                    return 2.0;
                } else {
                    return 4.0;
                }
            }

            vec3 FXAA(sampler2D tex, vec2 fragCoord, vec2 resolution)
            {
                vec2 inverseScreenSize = 1.0 / resolution;

                vec3 colorCenter = filteredSample(tex, fragCoord).rgb;

                float lumaCenter = rgb2luma(colorCenter);

                float lumaDown = rgb2luma(filteredSampleOffset(tex, fragCoord, ivec2(0, -1)).rgb);
                float lumaLeft = rgb2luma(filteredSampleOffset(tex, fragCoord, ivec2(-1, 0)).rgb);
                float lumaRight = rgb2luma(filteredSampleOffset(tex, fragCoord, ivec2(1, 0)).rgb);
                float lumaUp = rgb2luma(filteredSampleOffset(tex, fragCoord, ivec2(0, 1)).rgb);

                float lumaMin = min(lumaCenter, min(min(lumaDown, lumaLeft), min(lumaRight, lumaUp)));
                float lumaMax = max(lumaCenter, max(max(lumaDown, lumaLeft), max(lumaRight, lumaUp)));

                float lumaRange = lumaMax - lumaMin;

                const float EDGE_THRESHOLD_MIN = 0.0312;
                const float EDGE_THRESHOLD_MAX = 0.125;
                if (lumaRange < max(EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX))
                {
                    return colorCenter;
                }

                float lumaDownLeft = rgb2luma(filteredSampleOffset(tex, fragCoord, ivec2(-1, -1)).rgb);
                float lumaDownRight = rgb2luma(filteredSampleOffset(tex, fragCoord, ivec2(1, -1)).rgb);
                float lumaUpLeft = rgb2luma(filteredSampleOffset(tex, fragCoord, ivec2(-1, 1)).rgb);
                float lumaUpRight = rgb2luma(filteredSampleOffset(tex, fragCoord, ivec2(1, 1)).rgb);

                float lumaDownUp = lumaDown + lumaUp;
                float lumaLeftRight = lumaLeft + lumaRight;

                float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
                float lumaRightCorners = lumaDownRight + lumaUpRight;
                float lumaDownCorners = lumaDownLeft + lumaDownRight;
                float lumaUpCorners = lumaUpLeft + lumaUpRight;

                float edgeHorizontal =  abs(-2.0 * lumaLeft + lumaLeftCorners)  + abs(-2.0 * lumaCenter + lumaDownUp ) * 2.0    + abs(-2.0 * lumaRight + lumaRightCorners);
                float edgeVertical =    abs(-2.0 * lumaUp + lumaUpCorners)      + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0  + abs(-2.0 * lumaDown + lumaDownCorners);

                bool isHorizontal = (edgeHorizontal >= edgeVertical);

                float luma1 = isHorizontal ? lumaDown : lumaLeft;
                float luma2 = isHorizontal ? lumaUp : lumaRight;
                // Compute gradients in this direction.
                float gradient1 = luma1 - lumaCenter;
                float gradient2 = luma2 - lumaCenter;

                // Which direction is the steepest ?
                bool is1Steepest = abs(gradient1) >= abs(gradient2);

                // Gradient in the corresponding direction, normalized.
                float gradientScaled = 0.25*max(abs(gradient1),abs(gradient2));

                // Choose the step size (one pixel) according to the edge direction.
                float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;

                // Average luma in the correct direction.
                float lumaLocalAverage = 0.0;

                if(is1Steepest){
                    // Switch the direction
                    stepLength = - stepLength;
                    lumaLocalAverage = 0.5*(luma1 + lumaCenter);
                } else {
                    lumaLocalAverage = 0.5*(luma2 + lumaCenter);
                }

                // Shift UV in the correct direction by half a pixel.
                vec2 currentUv = fragCoord;
                if(isHorizontal){
                    currentUv.y += stepLength * 0.5;
                } else {
                    currentUv.x += stepLength * 0.5;
                }

                // Compute offset (for each iteration step) in the right direction.
                vec2 offset = isHorizontal ? vec2(inverseScreenSize.x,0.0) : vec2(0.0,inverseScreenSize.y);
                // Compute UVs to explore on each side of the edge, orthogonally. The QUALITY allows us to step faster.
                vec2 uv1 = currentUv - offset;
                vec2 uv2 = currentUv + offset;

                // Read the lumas at both current extremities of the exploration segment, and compute the delta wrt to the local average luma.
                float lumaEnd1 = rgb2luma(filteredSample(tex,uv1).rgb);
                float lumaEnd2 = rgb2luma(filteredSample(tex,uv2).rgb);
                lumaEnd1 -= lumaLocalAverage;
                lumaEnd2 -= lumaLocalAverage;

                // If the luma deltas at the current extremities are larger than the local gradient, we have reached the side of the edge.
                bool reached1 = abs(lumaEnd1) >= gradientScaled;
                bool reached2 = abs(lumaEnd2) >= gradientScaled;
                bool reachedBoth = reached1 && reached2;

                // If the side is not reached, we continue to explore in this direction.
                if(!reached1){
                    uv1 -= offset;
                }
                if(!reached2){
                    uv2 += offset;
                }

                const int ITERATIONS = 12;
                // If both sides have not been reached, continue to explore.
                if(!reachedBoth){

                    for(int i = 2; i < ITERATIONS; i++){
                        // If needed, read luma in 1st direction, compute delta.
                        if(!reached1){
                            lumaEnd1 = rgb2luma(filteredSample(tex, uv1).rgb);
                            lumaEnd1 = lumaEnd1 - lumaLocalAverage;
                        }
                        // If needed, read luma in opposite direction, compute delta.
                        if(!reached2){
                            lumaEnd2 = rgb2luma(filteredSample(tex, uv2).rgb);
                            lumaEnd2 = lumaEnd2 - lumaLocalAverage;
                        }
                        // If the luma deltas at the current extremities is larger than the local gradient, we have reached the side of the edge.
                        reached1 = abs(lumaEnd1) >= gradientScaled;
                        reached2 = abs(lumaEnd2) >= gradientScaled;
                        reachedBoth = reached1 && reached2;

                        // If the side is not reached, we continue to explore in this direction, with a variable quality.
                        if(!reached1){
                            uv1 -= offset * QUALITY(i);
                        }
                        if(!reached2){
                            uv2 += offset * QUALITY(i);
                        }

                        // If both sides have been reached, stop the exploration.
                        if(reachedBoth){ break;}
                    }
                }

                // Compute the distances to each extremity of the edge.
                float distance1 = isHorizontal ? (fragCoord.x - uv1.x) : (fragCoord.y - uv1.y);
                float distance2 = isHorizontal ? (uv2.x - fragCoord.x) : (uv2.y - fragCoord.y);

                // In which direction is the extremity of the edge closer ?
                bool isDirection1 = distance1 < distance2;
                float distanceFinal = min(distance1, distance2);

                // Length of the edge.
                float edgeThickness = (distance1 + distance2);

                // UV offset: read in the direction of the closest side of the edge.
                float pixelOffset = - distanceFinal / edgeThickness + 0.5;

                // Is the luma at center smaller than the local average ?
                bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;

                // If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive (same variation).
                // (in the direction of the closer side of the edge.)
                bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;

                // If the luma variation is incorrect, do not offset.
                float finalOffset = correctVariation ? pixelOffset : 0.0;

                const float SUBPIXEL_QUALITY = 0.75;

                // Sub-pixel shifting
                // Full weighted average of the luma over the 3x3 neighborhood.
                float lumaAverage = (1.0/12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
                // Ratio of the delta between the global average and the center luma, over the luma range in the 3x3 neighborhood.
                float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter)/lumaRange,0.0,1.0);
                float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
                // Compute a sub-pixel offset based on this delta.
                float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;

                // Pick the biggest of the two offsets.
                finalOffset = max(finalOffset,subPixelOffsetFinal);

                // Compute the final UV coordinates.
                vec2 finalUv = fragCoord;
                if(isHorizontal){
                    finalUv.y += finalOffset * stepLength;
                } else {
                    finalUv.x += finalOffset * stepLength;
                }

                // Read the color at the new UV coordinates, and use it.
                vec3 finalColor = filteredSample(tex,finalUv).rgb;

                return finalColor;
            }

            void main()
            {
                vec3 color = vec3(0.0);

                // FXAA
                if (u_useFXAA) {
                    color = FXAA(screenTexture, fragTexCoord, vec2(textureSize(screenTexture, 0)));
                }
                else {
                    color = filteredSample(screenTexture, fragTexCoord).rgb;
                }
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

    // create compute pipeline
    {
        // a test compute pass to transform the testRenderTextureImage, which is a texture that will be used to render view of a camera
        // we will will make a pixelation effect with this compute shader
        // the input image is read-write
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

    // This is mainly to move out the massive initialization code from this class
    TempResourceInitializer::init(resourceManager, renderer, desc);

    // Create a scene
    defaultScene = std::make_shared<Scene>();

    // set skybox
    {
        auto skybox = resourceManager.getTextureByName("skybox");
        defaultScene->setSkyboxTexture(skybox);
    }

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

            // render skybox
            {
                auto& skybox = sceneRenderData.skybox;
                auto mat = resourceManager.getMaterialByName("skyboxMaterial");
                mat->setTextureSampler("skybox", skybox.texture, 0);

                // mvp ubo
                struct SkyboxUniformBuffer
                {
                    glm::mat4 model = glm::mat4(1.0f);
                    glm::mat4 view = glm::mat4(1.0f);
                    glm::mat4 projection = glm::mat4(1.0f);
                } skyboxUniformBuffer {glm::mat4(1.0f), glm::inverse(glm::mat4_cast(mainCamera->getWorldTransform().getRotation())), camera.getCamera()->getProjection()};

                mat->setUniformBuffer("SkyboxUniformBuffer", &skyboxUniformBuffer, sizeof(skyboxUniformBuffer), 0);
                mat->use(renderer);
                auto mesh = resourceManager.getMeshByName("skyboxMesh");
                graphics::Renderable skyboxRenderable(mat->getMaterial(), mesh->getVertexData());
                renderer.draw(skyboxRenderable);
            }

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
