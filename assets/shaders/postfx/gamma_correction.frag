
#version 430 core

in vec2 fTexCoords;
out vec4 FragColor;

layout(binding = 0) uniform sampler2D frameTex;

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
    vec4 colorRGBA = texture(frameTex, fTexCoords);
    vec3 color = colorRGBA.rgb;

    color = ACESFilm(color); // tonne mapping
    color = pow(color, vec3(1.0 / 2.2)); // gamma correction

    FragColor = vec4(color, colorRGBA.w);
}


