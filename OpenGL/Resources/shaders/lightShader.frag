#version 330 core
out vec4 FragColor;

layout (std140) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
};

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    sampler2D texture_height1;
    sampler2D emission;
    float shininess;
};

struct SpotLight {
    vec3 position;
    vec3 direction;

    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 specular;
    vec3 diffuse;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in VS_OUT {
    vec3 Normal;
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    mat3 TBN;
} fs_in;

uniform Material material;

uniform DirLight dirLight;
#define NR_POINT_LIGHTS 1
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

uniform sampler2DArray shadowMap;
uniform bool usingNormalMap;
uniform bool usingHeightMap;

uniform float time;
uniform vec3 viewPos;
uniform int cascadeCount;
uniform mat4 view;
uniform float near;
uniform float far;
uniform bool shadows;
uniform float height_scale;

uniform float cascadePlaneDistances[16];

vec4 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow);
vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow);
vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

float ShadowCalculation(vec3 fragPosWorldSpace, vec3 normal) {
     // select cascade layer
    vec4 fragPosViewSpace = view * vec4(fragPosWorldSpace, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < cascadeCount; ++i)
    {
        if (depthValue < cascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = cascadeCount;
    }

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (currentDepth > 1.0)
    {
        return 0.0;
    }
    // calculate bias (based on depth map resolution and slope)

//    float bias = max(0.05 * (1.0 - dot(normal, dirLight.direction)), 0.005);
//    const float biasModifier = 0.5f;
//    if (layer == cascadeCount)
//    {
//        bias *= 1 / (far * biasModifier);
//    }
//    else
//    {
//        bias *= 1 / (cascadePlaneDistances[layer] * biasModifier);
//    }

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            //shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;  
            shadow += currentDepth > pcfDepth ? 1.0 : 0.0; 
        }    
    }
    shadow /= 9.0;
        
    return shadow;
}

float LinearizeDepth(float depth){
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    float height =  texture(material.texture_height1, texCoords).r;    
    vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
    return texCoords - p;
}

void main() {

    vec3 normal = normalize(fs_in.Normal);

    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    if(usingNormalMap) {
        normal = texture(material.texture_normal1, fs_in.TexCoords).rgb;
        normal = normalize(normal * 2.0 - 1.0);
    
        viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    }

    float shadow = 0.0;
    if (shadows) {
        shadow = ShadowCalculation(fs_in.FragPos, normal);
    }

    vec4 result = CalcDirLight(dirLight, normal, viewDir, shadow);

    //for(int i = 0; i < NR_POINT_LIGHTS; i++){
        //result += CalcPointLight(pointLights[i], norm, fs_in.FragPos, viewDir, shadow);
    //}

    //vec3 emission = vec3(0.0);
    //if (texture(material.texture_specular1, TexCoords).r == 0.0)
    //{
      //emission = texture(material.emission, TexCoords).rgb;
    //}

    //result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    //float depth = LinearizeDepth(gl_FragCoord.z) / far;

    //FragColor = vec4(vec3(depth), 1.0);
    FragColor = result;
}


vec4 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow) {
    vec2 texCoords = fs_in.TexCoords;
    vec3 lightDir = light.direction;

    if(usingHeightMap) {
        viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
        texCoords = ParallaxMapping(fs_in.TexCoords,  viewDir);
        //if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
            //discard;
    }

    // vec3 lightDir = fs_in.TBN * normalize(light.direction);

    vec3 halfWayDir = normalize(lightDir + viewDir);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    //vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal, halfWayDir), 0.0), material.shininess);

    if(diff == 0.0){
        spec = 0.0;
    }
    // combine results
    vec4 ambient  = vec4(light.ambient.rgb, 1.0)  * texture(material.texture_diffuse1, texCoords);
    vec4 diffuse  = vec4(light.diffuse.rgb, 1.0)  * diff * texture(material.texture_diffuse1, texCoords);
    vec4 specular = vec4(light.specular.rgb, 1.0) * spec * texture(material.texture_specular1, texCoords);

    if (shadows) {
        return (ambient + (1.0 - shadow) * (diffuse + specular));
    }
    return (ambient + diffuse + specular);
}

vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow) {
    vec2 texCoords = fs_in.TexCoords;

    if(usingHeightMap) {
        viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
        texCoords = ParallaxMapping(fs_in.TexCoords,  viewDir);
        if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
            discard;
    }

    vec3 lightDir = fs_in.TBN * normalize(light.position - fragPos);
    vec3 halfWayDir = normalize(lightDir + viewDir);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
   // vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal, halfWayDir), 0.0), material.shininess);

    if(diff == 0.0){
        spec = 0.0;
    }
   
   // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));

    // combine results
    vec4 ambient  = vec4(light.ambient.xyz, 1.0)  * texture(material.texture_diffuse1, texCoords);
    vec4 diffuse  = vec4(light.diffuse.xyz, 1.0)  * diff * texture(material.texture_diffuse1, texCoords);
    vec4 specular = vec4(light.specular.xyz, 1.0) * spec * texture(material.texture_specular1, texCoords);

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + (1.0 - shadow) * (diffuse + specular));
}

vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = fs_in.TBN * normalize(light.position - fragPos);
    vec3 halfWayDir = normalize(lightDir + viewDir);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    //vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal, halfWayDir), 0.0), material.shininess);

    if(diff == 0.0){
        spec = 0.0;
    }

    // combine results
    vec4 ambient  = vec4(light.ambient.xyz, 1.0)  * texture(material.texture_diffuse1, fs_in.TexCoords);
    vec4 diffuse  = vec4(light.diffuse.xyz, 1.0)  * diff * texture(material.texture_diffuse1, fs_in.TexCoords);
    vec4 specular = vec4(light.specular.xyz, 1.0) * spec * texture(material.texture_specular1, fs_in.TexCoords);

    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    diffuse *= intensity;
    specular *= intensity;

    return (ambient + diffuse + specular);
}