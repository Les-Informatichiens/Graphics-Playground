#version 450

            const int MAX_LIGHTS = 10;


layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;
//            layout(location = 3) in vec3 inTangent;
//            layout(location = 4) in vec3 inBitangent;

layout (location = 0) out vec2 fragTexCoord;
layout (location = 1) out vec3 fragWorldPos;
layout (location = 2) out vec3 fragNormal;

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

layout (binding = 6, std140) uniform Lights {
    int lightCount;
    Light lights[MAX_LIGHTS];
} pointLights;


layout (binding = 7, std140) uniform Spotlights {
    int lightCount;
    Spotlight lights[MAX_LIGHTS];
} spotlights;

struct DirectionalLight {
    vec3 color;
    vec3 direction;
    float intensity;
};

layout (binding = 8, std140) uniform DirectionalLights {
    DirectionalLight directionalLight;
} directionalLights;


layout (binding = 1) uniform Constants {
    vec3 cameraPos;
    vec3 cameraDir;
    vec3 unused_;
    int lightModel;
} constants;

layout (binding = 2, std140) uniform Settings {
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

layout (binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout (location = 3) out vec3 gouraudColor;

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
        vec3 albedo = settings.baseColor;
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