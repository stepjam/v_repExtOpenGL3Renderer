#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirLight {
    vec3 position; // specifically for shadows
    vec3 direction;

//    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    mat4 lightSpaceMatrix;
    sampler2D shadowMap;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

//    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float farPlane;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

//    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    mat4 lightSpaceMatrix;
    sampler2D shadowMap;
};

#define MAX_LIGHTS 10

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture0;

uniform vec3 sceneAmbient;
uniform vec3 viewPos;
uniform DirLight dirLights[MAX_LIGHTS];
uniform PointLight pointLights[MAX_LIGHTS];
uniform SpotLight spotLights[MAX_LIGHTS];

// For some reason, we are not allowed to store samplerCube in structs or arrays
// So allow a max of 5
uniform samplerCube depthCubeMap0;
uniform samplerCube depthCubeMap1;
uniform samplerCube depthCubeMap2;
uniform samplerCube depthCubeMap3;
uniform samplerCube depthCubeMap4;

uniform int dirLightLen;
uniform int pointLightLen;
uniform int spotLightLen;

uniform Material material;

// array of offset direction for sampling in pointlight shadows
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1),
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float ShadowCalculation(mat4 lightSpaceMatrix, vec3 lightPos, sampler2D shadowMap);
float PointShadowCalculation(vec3 lightPos, samplerCube cubeMap, float farPlane);

void main()
{

    // Lighting properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Give it some intial ambient light
    vec3 ambient = sceneAmbient * vec3(texture(texture0, TexCoords)) * material.ambient;

    vec3 result = vec3(0.0);

    // phase 1: directional lighting
    for(int i = 0; i < dirLightLen; i++)
    {
        vec3 lresult = CalcDirLight(dirLights[i], norm, viewDir);
        float shadow = ShadowCalculation(dirLights[i].lightSpaceMatrix,
                                         dirLights[i].position,
                                         dirLights[i].shadowMap);
        result += lresult * (1.0 - shadow);
    }
    // phase 2: point lights
    for(int i = 0; i < pointLightLen; i++)
    {
        vec3 lresult = CalcPointLight(pointLights[i], norm, FragPos, viewDir);
        float shadow = 0.0;

        if (i == 0)
            shadow = PointShadowCalculation(pointLights[i].position, depthCubeMap0, pointLights[i].farPlane);
        else if (i == 1)
            shadow = PointShadowCalculation(pointLights[i].position, depthCubeMap1, pointLights[i].farPlane);
        else if (i == 2)
            shadow = PointShadowCalculation(pointLights[i].position, depthCubeMap2, pointLights[i].farPlane);
        else if (i == 3)
            shadow = PointShadowCalculation(pointLights[i].position, depthCubeMap3, pointLights[i].farPlane);
        else if (i == 4)
            shadow = PointShadowCalculation(pointLights[i].position, depthCubeMap4, pointLights[i].farPlane);

        result += lresult * (1.0 - shadow);
    }
    // phase 3: spot light
    for(int i = 0; i < spotLightLen; i++)
    {
        vec3 lresult = CalcSpotLight(spotLights[i], norm, FragPos, viewDir);
        float shadow = ShadowCalculation(spotLights[i].lightSpaceMatrix,
                                         spotLights[i].position,
                                         spotLights[i].shadowMap);
        result += lresult * (1.0 - shadow);
    }

    vec3 final = ambient + result; //(1.0 - shadow) * result;
    FragColor = vec4(final, 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading (Phong)
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // specular shading (Blinn-Phong)
    // vec3 halfwayDir = normalize(lightDir + viewDir);
    // float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    // combine results
    vec3 diffuse = light.diffuse * (diff * (vec3(texture(texture0, TexCoords)) * material.diffuse));
    vec3 specular = light.specular * (spec * (vec3(texture(texture0, TexCoords)) * material.specular));
    return (diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // combine results
    vec3 diffuse = light.diffuse * (diff * material.diffuse);
    vec3 specular = light.specular * (spec * material.specular);

    diffuse *= attenuation;
    specular *= attenuation;
    return (diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);

    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results

    vec3 diffuse = light.diffuse * (diff * material.diffuse);
    vec3 specular = light.specular * (spec * material.specular);

    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (diffuse + specular);
}

float ShadowCalculation(mat4 lightSpaceMatrix, vec3 lightPos, sampler2D shadowMap)
{
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float bias = max(0.01 * (1.0 - dot(normal, lightDir)), 0.001);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

float PointShadowCalculation(vec3 lightPos, samplerCube cubeMap, float farPlane)
{

    // get vector between fragment position and light position
    vec3 fragToLight = FragPos - lightPos;
    float currentDepth = length(fragToLight);

    float shadow = 0.0;

    // Bias calc
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float bias = max(0.02 * (1.0 - dot(normal, lightDir)), 0.001);

    int samples = 20;
    float viewDistance = length(viewPos - FragPos);
    float diskRadius = (1.0 + (viewDistance / farPlane)) / 500.0;

    for(int i = 0; i < samples; i++)
    {
        float closestDepth = texture(cubeMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= farPlane;   // undo mapping [0;1]
        if((currentDepth - bias) > closestDepth){
            shadow += 1.0;
        }
    }
    shadow /= float(samples);
    return shadow;
}
