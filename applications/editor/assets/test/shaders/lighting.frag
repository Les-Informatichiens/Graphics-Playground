#version 450

            const int MAX_LIGHTS = 10;

layout (location = 0) in vec2 fragTexCoord;
layout (location = 1) in vec3 fragWorldPos;
layout (location = 2) in vec3 fragNormal;
layout (location = 3) in vec3 gouraudColor;

out vec4 fragColor;

layout (binding = 1) uniform sampler2D albedoMap;
layout (binding = 2) uniform sampler2D normalMap;
layout (binding = 3) uniform sampler2D metallicMap;
layout (binding = 4) uniform sampler2D roughnessMap;
layout (binding = 5) uniform sampler2D aoMap;
layout (binding = 6) uniform sampler2D emissiveMap;

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
    map = map * 255. / 127. - 128. / 127.;
    mat3 TBN = cotangentFrame(N, -V, uv);

    //map.y = -map.y;

    return normalize(TBN * map);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


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

vec3 calcPhong(vec3 N, vec3 V, vec3 L, vec3 color, vec3 albedo, float intensity, float shininess)
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

vec3 calcToon(vec3 N, vec3 V, vec3 L, vec3 color, vec3 albedo, float intensity, float threshold, float blur)
{
    vec3 lightDir = normalize(L);
    vec3 normal = normalize(N);
    float NdotL = max(dot(lightDir, normal), 0.0);

    float cel = 0.0;

    if (NdotL < threshold - blur / 2.0) {
        cel = 0.0;
    }
    else if (NdotL > threshold + blur / 2.0) {
        cel = 1.0;
    }
    else {
        cel = 1.0 - ((threshold + blur / 2.0 - NdotL) / blur);
    }

    vec3 diffuse = albedo * ((cel + 0.3) / 2.5) * color * intensity;

    return diffuse;
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

    vec3 albedo = settings.useAlbedoMap ? pow(texture(albedoMap, fragTexCoord).rgb, vec3(2.2)) : vec3(1.0);
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

        color += emission;
    }
    else if (constants.lightModel == 2) {
        vec3 Lo = vec3(0.0);

        for (int i = 0; i < 1; ++i) {
            Lo += calcPhong(N, V, directionalLights.directionalLight.direction, directionalLights.directionalLight.color, material.albedo, directionalLights.directionalLight.intensity, 32.0);
        }

        for (int i = 0; i < pointLights.lightCount; ++i) {
            Light light = pointLights.lights[i];
            float intensity = calcPointLightIntensity(light.position, fragWorldPos, light.constant, light.linear, light.quadratic);
            Lo += calcPhong(N, V, light.position - fragWorldPos, light.color, material.albedo, light.intensity, 32.0) * intensity;
        }

        for (int i = 0; i < spotlights.lightCount; ++i) {
            Spotlight light = spotlights.lights[i];
            float intensity = calcSpotlightIntensity(light.position, light.direction, fragWorldPos, light.constant, light.linear, light.quadratic, light.cutoff, light.outerCutoff);
            Lo += calcPhong(N, V, light.position - fragWorldPos, light.color, material.albedo, light.intensity, 32.0) * intensity;
        }

        vec3 ambient = vec3(0.00) * material.albedo * material.ao;

        color = ambient + Lo;
        color += emission;
    }
    else if (constants.lightModel == 3) {
        color = gouraudColor;// * material.albedo;
    }
    // toon
    else if (constants.lightModel == 4) {
        vec3 Lo = vec3(0.0);

        float blur = 0.1;
        float threshold = 0.5;

        for (int i = 0; i < 1; ++i) {
            Lo += calcToon(N, V, directionalLights.directionalLight.direction, directionalLights.directionalLight.color, material.albedo, directionalLights.directionalLight.intensity, threshold, blur);
        }

        for (int i = 0; i < pointLights.lightCount; ++i) {
            Light light = pointLights.lights[i];
            float intensity = calcPointLightIntensity(light.position, fragWorldPos, light.constant, light.linear, light.quadratic);
            Lo += calcToon(N, V, light.position - fragWorldPos, light.color, material.albedo, light.intensity, threshold, blur) * intensity;
        }

        for (int i = 0; i < spotlights.lightCount; ++i) {
            Spotlight light = spotlights.lights[i];
            float intensity = calcSpotlightIntensity(light.position, light.direction, fragWorldPos, light.constant, light.linear, light.quadratic, light.cutoff, light.outerCutoff);
            Lo += calcToon(N, V, light.position - fragWorldPos, light.color, material.albedo, light.intensity, threshold, blur) * intensity;
        }

        vec3 ambient = vec3(0.00) * material.albedo * material.ao;

        color = ambient + Lo;
        color += emission;
    }


    color.r = max(color.r, 0.0);
    color.g = max(color.g, 0.0);
    color.b = max(color.b, 0.0);

    fragColor = vec4(color, 1.0);
}