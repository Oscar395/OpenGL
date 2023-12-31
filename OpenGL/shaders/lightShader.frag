#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
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
    vec4 FragPosLightSpace;
} fs_in;

uniform Material material;

uniform DirLight dirLight;
#define NR_POINT_LIGHTS 1
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

uniform sampler2D shadowMap;

uniform float time;
uniform vec3 lightPos;
uniform vec3 viewPos;

vec4 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow);
vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow);
vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

float near = 0.1;
float far = 100.0;

float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    //float closetDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    //float shadow = currentDepth - bias > closetDepth ? 1.0 : 0.0;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    if (projCoords.z > 1.0) {
        shadow = 0.0;
    }
    return shadow;
}

float LinearizeDepth(float depth){
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {

    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);

    vec4 result = CalcDirLight(dirLight, norm, viewDir, shadow);

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
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
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
    vec4 ambient  = vec4(light.ambient.rgb, 1.0)  * texture(material.texture_diffuse1, fs_in.TexCoords);
    vec4 diffuse  = vec4(light.diffuse.rgb, 1.0)  * diff * texture(material.texture_diffuse1, fs_in.TexCoords);
    vec4 specular = vec4(light.specular.rgb, 1.0) * spec * texture(material.texture_specular1, fs_in.TexCoords);

    return (ambient + (1.0 - shadow) * (diffuse + specular));
}

vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow) {
    vec3 lightDir = normalize(light.position - fragPos);
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
    vec4 ambient  = vec4(light.ambient.xyz, 1.0)  * texture(material.texture_diffuse1, fs_in.TexCoords);
    vec4 diffuse  = vec4(light.diffuse.xyz, 1.0)  * diff * texture(material.texture_diffuse1, fs_in.TexCoords);
    vec4 specular = vec4(light.specular.xyz, 1.0) * spec * texture(material.texture_specular1, fs_in.TexCoords);

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + (1.0 - shadow) * (diffuse + specular));
}

vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
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