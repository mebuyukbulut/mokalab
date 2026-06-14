#version 420 core

in vec3 fPos;
in vec3 fNormal;
in vec2 fTexCoords;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;


uniform sampler2D baseColorMap;
uniform sampler2D armMap;
uniform sampler2D normalMap;
uniform sampler2D emissiveMap;

uniform sampler2D shadowMap;

uniform mat4 lightSpaceMatrix;

// PBR0.frag veya ilgili shader dosyası
struct Light {
    vec4 position;  // xyz + 1 float padding
    vec4 color;     // rgb + intensity (intensity'yi dördüncü kanal yapabilirsin)
    vec4 params;    // x: attenuation, y: type, z: cutoff, w: padding
    vec4 direction; // xyz + 1 float padding
};

layout (std140, binding = 1) uniform LightBlock {
    Light lights[8];  // Maksimum ışık sayısı (NR_LIGHTS)
    int numLights;    // Aktif ışık sayısı
} ubo_data;



layout(std140, binding = 0) uniform FrameUniforms
{
    mat4 view;
    mat4 projection;

    vec3 viewPos;
    float _pad0;
};


//struct Light {    
//    vec3 position;
//	vec3 color;
//	float intensity;
//    float attenuation;
//    int type; // 0: point, 1: spot, 2: directional
//    vec3 direction; // only for dir and spot
//    float cutoff; // only for spot
//};  
//#define NR_LIGHTS 8  
//uniform Light _lights[NR_LIGHTS];
//uniform int numLights;

uniform float ambientIntensity;
uniform vec3 ambientColor;


struct Material{
	vec4 baseColor;
	vec4 emissive;
	float metallic;
	float roughness;
	float reflectance;
	float ao;
};
uniform Material material;

struct SurfaceData
{
    vec3 baseColor;
    float ao;
    float roughness;
    float metallic;
    float reflectance;
    vec3 emissive;
    vec3 normal;
    vec3 viewDir;
};


#define PI 3.1415926535897932384626433832795

float near = 0.1; // near plane    
float far = 100.0; // far plane

float D_GGX(float NoH, float roughness) {
    float a2     = roughness*roughness;
    float NdotH  = NoH;
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;


//    float a = NoH * roughness;
//    float k = roughness / (1.0 - NoH * NoH + a * a);
//    return k * k * (1.0 / PI);
}
//// ----------------------------------------------------------------------------
//float GeometrySchlickGGX(float NdotV, float roughness)
//{
//    float r = (roughness + 1.0);
//    float k = (r*r) / 8.0;
//
//    float nom   = NdotV;
//    float denom = NdotV * (1.0 - k) + k;
//
//    return nom / denom;
//}
float V_SmithGGXCorrelated(float NoV, float NoL, float roughness) {
    float a2 = roughness * roughness;
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);

//    float ggx2 = GeometrySchlickGGX(NoV, roughness);
//    float ggx1 = GeometrySchlickGGX(NoL, roughness);
//
//    return ggx1 * ggx2;
}
vec3 F_Schlick(float u, vec3 f0) {
    return f0 + (1 - f0) * pow(1.0 - u, 5.0);
}
float F_Schlick(float u, float f0, float f90) {
    return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}
float Fd_Burley(float NoV, float NoL, float LoH, float roughness) {
    float f90 = 0.5 + 2.0 * roughness * LoH * LoH;
    float lightScatter = F_Schlick(NoL, 1.0, f90);
    float viewScatter = F_Schlick(NoV, 1.0, f90);
    return lightScatter * viewScatter * (1.0 / PI);
}


float Fd_Lambert() {
    return 1.0 / PI;
}

vec3 CalcPointLight(Light light, SurfaceData s)
{
    float attRad = light.params.x; 

    vec3 diffuseColor = (1.0 - s.metallic) * s.baseColor.rgb;
    vec3 f0 = 0.16 * s.reflectance * s.reflectance * (1.0 - s.metallic) + s.baseColor.rgb * s.metallic;
  
    vec3 n = normalize(fNormal); // Surface normal vector
    vec3 l = normalize(light.position.xyz - fPos ); // Incident light vector
    vec3 v = normalize(viewPos - fPos); // View/Eye vector
    vec3 h = normalize((v+l)/2); // halfway 
    
    //+ 1e-5; // this create black dots on model maybe we can need this parameter in future but now I remove it 
    float NoV = abs(dot(n,v)) ; //+ 1e-5; // <- this parameter.
    float NoL = clamp(dot(n, l), 0.0, 1.0);
    float NoH = clamp(dot(n, h), 0.0, 1.0);
    float LoH = clamp(dot(l, h), 0.0, 1.0);
    

    float D = D_GGX(NoH, s.roughness);
    vec3  F = F_Schlick(LoH, f0);
    float V = V_SmithGGXCorrelated(NoV, NoL, s.roughness);

    // specular BRDF
    vec3 Fr = (D * V) * F;

    // diffuse BRDF
    vec3 Fd = diffuseColor * Fd_Burley(NoV, NoL, LoH, s.roughness) * (1.0 - F);


    // attenuation
    float distance    = max(length(light.position.xyz - fPos), 0.01);
    float distance2    = distance * distance; // squared distance 
    
    float E = 1.0 / (distance2 * PI); // inverse square law and 1/PI
    float window = 1.0f - (distance2 * distance2 / pow(attRad, 4.0)) ; 
    E *= pow(clamp(window, 0.0, 1.0), 2.0); 

    return (Fd + Fr) * light.color.w * light.color.xyz * NoL * E;
} 

vec3 CalcDirectionalLight(Light light, SurfaceData s)
{
    vec3 diffuseColor = (1.0 - s.metallic) * s.baseColor.rgb;
    vec3 f0 = 0.16 * s.reflectance * s.reflectance * (1.0 - s.metallic) + s.baseColor.rgb * s.metallic;
  
    vec3 n = normalize(fNormal); // Surface normal vector
    vec3 l = normalize(light.direction.xyz); // Incident light vector
    vec3 v = normalize(viewPos - fPos); // View/Eye vector
    vec3 h = normalize((v+l)/2); // halfway 
    
    //+ 1e-5; // this create black dots on model maybe we can need this parameter in future but now I remove it 
    float NoV = abs(dot(n,v)) ; //+ 1e-5; // <- this parameter.
    float NoL = clamp(dot(n, l), 0.0, 1.0);
    float NoH = clamp(dot(n, h), 0.0, 1.0);
    float LoH = clamp(dot(l, h), 0.0, 1.0);
    

    float D = D_GGX(NoH, s.roughness);
    vec3  F = F_Schlick(LoH, f0);
    float V = V_SmithGGXCorrelated(NoV, NoL, s.roughness);

    // specular BRDF
    vec3 Fr = (D * V) * F;

    // diffuse BRDF
    vec3 Fd = diffuseColor * Fd_Burley(NoV, NoL, LoH, s.roughness) * (1.0 - F);


    // attenuation
//    float distance    = max(length(light.position - fPos), 0.01);
//    float distance2    = distance * distance; // squared distance 
//    
    //float E = 1.0 / (distance2 * PI); // inverse square law and 1/PI
    //float window = 1.0f - (distance2 * distance2 / pow(attRad, 4.0)) ; 
    //E *= pow(clamp(window, 0.0, 1.0), 2.0); 





    // Shadow calculations
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    //vec3 n = normalize(fNormal); // Surface normal vector
    //vec3 l = lightDir; //normalize(ubo_data.lights[0].position.xyz - fPos ); // Incident light vector
    float bias = 0.005 * tan(acos(NoL));

    float shadow = 0.0;
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
    projCoords.y < 0.0 || projCoords.y > 1.0 ||
    projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        shadow = 0.0;
    }
    else{
        //shadow =
        //currentDepth - bias >= closestDepth
        //? 1.0
        //: 0.0;

        shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        for(int x = -1; x <= 1; x+=2)
        {
            for(int y = -1; y <= 1; y+=2)
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= 4.0;
    }

//    float shadow = 0.0;
//    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
//    for(int x = -1; x <= 1; ++x)
//    {
//        for(int y = -1; y <= 1; ++y)
//        {
//            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
//            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
//        }    
//    }
//    shadow /= 9.0;
    

    
    //vec3 result = pLights * (1.0 - shadow);// * material.color;

    return (Fd + Fr) * light.color.w * light.color.xyz * NoL * (1.0 - shadow); // * E;
} 


vec3 CalcSpotLight(Light light, SurfaceData s){
    vec3 lightDir =  normalize(fPos - light.position.xyz);
    float theta = dot(lightDir, normalize(light.direction.xyz));
    float phi = cos(radians(light.params.z));

    if(theta < phi) 
      return vec3(0,0,0); 


    float attRad = light.params.x;

    vec3 diffuseColor = (1.0 - s.metallic) * s.baseColor.rgb;
    vec3 f0 = 0.16 * s.reflectance * s.reflectance * (1.0 - s.metallic) + s.baseColor.rgb * s.metallic;
  
    vec3 n = normalize(fNormal); // Surface normal vector
    vec3 l = normalize(light.position.xyz - fPos ); // Incident light vector
    vec3 v = normalize(viewPos - fPos); // View/Eye vector
    vec3 h = normalize((v+l)/2); // halfway 
    
    //+ 1e-5; // this create black dots on model maybe we can need this parameter in future but now I remove it 
    float NoV = abs(dot(n,v)) ; //+ 1e-5; // <- this parameter.
    float NoL = clamp(dot(n, l), 0.0, 1.0);
    float NoH = clamp(dot(n, h), 0.0, 1.0);
    float LoH = clamp(dot(l, h), 0.0, 1.0);
    

    float D = D_GGX(NoH, s.roughness);
    vec3  F = F_Schlick(LoH, f0);
    float V = V_SmithGGXCorrelated(NoV, NoL, s.roughness);

    // specular BRDF
    vec3 Fr = (D * V) * F;

    // diffuse BRDF
    vec3 Fd = diffuseColor * Fd_Burley(NoV, NoL, LoH, s.roughness) * (1.0 - F);


    // attenuation
    float distance    = max(length(light.position.xyz - fPos), 0.01);
    float distance2    = distance * distance; // squared distance 
    
    float E = 1.0 / (distance2 * PI); // inverse square law and 1/PI
    float window = 1.0f - (distance2 * distance2 / pow(attRad, 4.0)) ; 
    E *= pow(clamp(window, 0.0, 1.0), 2.0); 



    float angleWindow = 1 - (pow (1 - theta, 4) / pow(1 - phi, 4)); 
    E*= angleWindow; 





    return (Fd + Fr) * light.color.w * light.color.xyz * NoL * E;
} 


float LinearizeDepth(float depth) {
    // Convert depth from NDC to linear depth
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

SurfaceData buildSurfaceData(){
    SurfaceData s;

    vec4 baseColorSample = texture(baseColorMap, fTexCoords);
    vec3 armSample = texture(armMap, fTexCoords).rgb;
    vec3 emissiveSample = texture(emissiveMap, fTexCoords).rgb;


    float perceptualRoughness = armSample.g * material.roughness;;  
    float roughness = clamp(pow(perceptualRoughness,2), pow(0.01,2), 1) ; 

    s.baseColor = (baseColorSample * material.baseColor).rgb;
    s.ao = armSample.r * material.ao;
    s.roughness = roughness;
    s.metallic = armSample.b * material.metallic;
    s.reflectance = material.reflectance;
    s.emissive = emissiveSample * material.emissive.rgb;

    s.normal = normalize(fNormal);
    s.viewDir = normalize(viewPos - fPos);

    return s;
}

void main(){
	SurfaceData s = buildSurfaceData();

    vec3 pLights = vec3(0.0);
    for(int i = 0; i < ubo_data.numLights; i++){
        int lightType = int(ubo_data.lights[i].params.y);

        if(lightType == 1) pLights += CalcDirectionalLight(ubo_data.lights[i], s);
        if(lightType == 2) pLights += CalcPointLight(ubo_data.lights[i], s);
        if(lightType == 3) pLights += CalcSpotLight(ubo_data.lights[i], s);
    }

    vec3 result = pLights;

    FragColor = vec4(result,1.0);

    // check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722)); // convert grayscale
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}	
