#version 330 core

struct Material {
    vec4 ambient;
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
    float bias;
    float normalBias;
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
    float bias;
    float normalBias;
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
    float bias;
    float normalBias;
};

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture0;
uniform int textureApplyMode;

uniform vec3 sceneAmbient;
uniform vec3 viewPos;

// Some graphics cards appear not to support arrays. One way around this is to explicitly roll out the array.
#define MAX_LIGHTS 5
uniform DirLight dirLight0;
uniform DirLight dirLight1;
uniform DirLight dirLight2;
uniform DirLight dirLight3;
uniform DirLight dirLight4;

uniform PointLight pointLight0;
uniform PointLight pointLight1;
uniform PointLight pointLight2;
uniform PointLight pointLight3;
uniform PointLight pointLight4;

uniform SpotLight spotLight0;
uniform SpotLight spotLight1;
uniform SpotLight spotLight2;
uniform SpotLight spotLight3;
uniform SpotLight spotLight4;

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
float ShadowCalculation(mat4 lightSpaceMatrix, vec3 lightPos, sampler2D shadowMap, float bias, float normalBias);
float PointShadowCalculation(vec3 lightPos, samplerCube cubeMap, float farPlane, float bias, float normalBias);

void main()
{
    // Lighting properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Give it some intial ambient light
    vec4 ambient = texture(texture0, TexCoords);

    // textureApplyMode 1 (GL_DECAL) gets applied later.
    if (textureApplyMode == 2)  // ADD
        ambient = (ambient + material.ambient, 1.0) * vec4(sceneAmbient, 1.0);
    else if (textureApplyMode == 1)
        ambient = material.ambient;
    else if (textureApplyMode != 1)
        // In this case, we either have no texture, or we want to use GL_BLEND.
        ambient *= material.ambient * vec4(sceneAmbient, 1.0);

    vec3 result = vec3(0.0);
    // phase 1: directional lighting
    for(int i = 0; i < dirLightLen && i < MAX_LIGHTS; i++)
    {
        vec3 lresult;
        float shadow = 0.0;
        // We are not allowed to store the struct in a temp, apparently.
        if (i == 0){
            lresult = CalcDirLight(dirLight0, norm, viewDir);
            shadow = ShadowCalculation(dirLight0.lightSpaceMatrix, dirLight0.position, dirLight0.shadowMap, dirLight0.bias, dirLight0.normalBias);
        } else if (i == 1){
            lresult = CalcDirLight(dirLight1, norm, viewDir);
            shadow = ShadowCalculation(dirLight1.lightSpaceMatrix, dirLight1.position, dirLight1.shadowMap, dirLight1.bias, dirLight1.normalBias);
        } else if (i == 2){
            lresult = CalcDirLight(dirLight2, norm, viewDir);
            shadow = ShadowCalculation(dirLight2.lightSpaceMatrix, dirLight2.position, dirLight2.shadowMap, dirLight2.bias, dirLight2.normalBias);
        } else if (i == 3){
            lresult = CalcDirLight(dirLight3, norm, viewDir);
            shadow = ShadowCalculation(dirLight3.lightSpaceMatrix, dirLight3.position, dirLight3.shadowMap, dirLight3.bias, dirLight3.normalBias);
        } else if (i == 4){
            lresult = CalcDirLight(dirLight4, norm, viewDir);
            shadow = ShadowCalculation(dirLight4.lightSpaceMatrix, dirLight4.position, dirLight4.shadowMap, dirLight4.bias, dirLight4.normalBias);
        }
        result += lresult * (1.0 - shadow);
    }
    // phase 2: point lights
    for(int i = 0; i < pointLightLen && i < MAX_LIGHTS; i++)
    {
        vec3 lresult;
        float shadow = 0.0;

        if (i == 0){
            lresult = CalcPointLight(pointLight0, norm, FragPos, viewDir);
            shadow = PointShadowCalculation(pointLight0.position, depthCubeMap0, pointLight0.farPlane, pointLight0.bias, pointLight0.normalBias);
        }else if (i == 1){
            lresult = CalcPointLight(pointLight1, norm, FragPos, viewDir);
            shadow = PointShadowCalculation(pointLight1.position, depthCubeMap1, pointLight1.farPlane, pointLight1.bias, pointLight1.normalBias);
        }else if (i == 2){
            lresult = CalcPointLight(pointLight2, norm, FragPos, viewDir);
            shadow = PointShadowCalculation(pointLight2.position, depthCubeMap2, pointLight2.farPlane, pointLight2.bias, pointLight2.normalBias);
        }else if (i == 3){
            lresult = CalcPointLight(pointLight3, norm, FragPos, viewDir);
            shadow = PointShadowCalculation(pointLight3.position, depthCubeMap3, pointLight3.farPlane, pointLight3.bias, pointLight3.normalBias);
        }else if (i == 4){
            lresult = CalcPointLight(pointLight4, norm, FragPos, viewDir);
            shadow = PointShadowCalculation(pointLight4.position, depthCubeMap4, pointLight4.farPlane, pointLight4.bias, pointLight4.normalBias);
        }
        result += lresult * (1.0 - shadow);
    }
    // phase 3: spot light
    for(int i = 0; i < spotLightLen && i < MAX_LIGHTS; i++)
    {
        vec3 lresult;
        float shadow = 0.0;
        // We are not allowed to store the struct in a temp, apparently.
        if (i == 0){
            lresult = CalcSpotLight(spotLight0, norm, FragPos, viewDir);
            shadow = ShadowCalculation(spotLight0.lightSpaceMatrix, spotLight0.position, spotLight0.shadowMap, spotLight0.bias, spotLight0.normalBias);
        } else if (i == 1){
            lresult = CalcSpotLight(spotLight1, norm, FragPos, viewDir);
            shadow = ShadowCalculation(spotLight1.lightSpaceMatrix, spotLight1.position, spotLight1.shadowMap, spotLight1.bias, spotLight1.normalBias);
        } else if (i == 2){
            lresult = CalcSpotLight(spotLight2, norm, FragPos, viewDir);
            shadow = ShadowCalculation(spotLight2.lightSpaceMatrix, spotLight2.position, spotLight2.shadowMap, spotLight2.bias, spotLight2.normalBias);
        } else if (i == 3){
            lresult = CalcSpotLight(spotLight3, norm, FragPos, viewDir);
            shadow = ShadowCalculation(spotLight3.lightSpaceMatrix, spotLight3.position, spotLight3.shadowMap, spotLight3.bias, spotLight3.normalBias);
        } else if (i == 4){
            lresult = CalcSpotLight(spotLight4, norm, FragPos, viewDir);
            shadow = ShadowCalculation(spotLight4.lightSpaceMatrix, spotLight4.position, spotLight4.shadowMap, spotLight4.bias, spotLight4.normalBias);
        }
        result += lresult * (1.0 - shadow);
    }

    FragColor = ambient + vec4(result, 0.0);
    if (textureApplyMode == 1){
        // textureApplyMode == 1 is DECAL
        vec4 coltex = texture(texture0, TexCoords.st);
        vec3 col = FragColor.rgb * (1.0-coltex.a) + coltex.rgb * coltex.a;
        FragColor = vec4(col, FragColor.a);
    }
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

float ShadowCalculation(mat4 lightSpaceMatrix, vec3 lightPos, sampler2D shadowMap, float bias, float normalBias)
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
    bias = max(normalBias * (1.0 - dot(normal, lightDir)), bias);

    // check whether current frag pos is in shadow
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

float PointShadowCalculation(vec3 lightPos, samplerCube cubeMap, float farPlane, float bias, float normalBias)
{

    // get vector between fragment position and light position
    vec3 fragToLight = FragPos - lightPos;
    float currentDepth = length(fragToLight);

    float shadow = 0.0;

    // Bias calc
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    bias = max(normalBias * (1.0 - dot(normal, lightDir)), bias);

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
