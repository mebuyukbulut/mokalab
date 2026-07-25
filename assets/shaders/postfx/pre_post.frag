#version 430 core

in vec2 fTexCoords;
out vec4 FragColor;

layout(binding = 0) uniform sampler2D frameTex0; // lit
layout(binding = 1) uniform sampler2D frameTex1; // bloom
layout(binding = 2) uniform sampler2D frameTex2; // bg

uniform bool bloomEnable = false;
uniform bool postProcEnable = true;

vec3 ACESFilm(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14; 
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0); 
}

void main(){

    vec4 hdrColor   = texture(frameTex0, fTexCoords);
    vec4 bloomColor = texture(frameTex1, fTexCoords);
    vec4 bgColor    = texture(frameTex2, fTexCoords);


    if(bloomEnable) 
        hdrColor.rgb += bloomColor.rgb; // bloom disabled for now


    vec3 color = hdrColor.rgb;

    if(postProcEnable){
        color = ACESFilm(color); // tonne mapping
        color = pow(color, vec3(1.0 / 2.2)); // gamma correction        
    }
    float oran = hdrColor.a; 
    if(bloomEnable)
        oran = max(hdrColor.a, bloomColor.a);
    vec3 composite = mix(bgColor.rgb, color, oran);

    FragColor = vec4(composite, 1.0f);
}