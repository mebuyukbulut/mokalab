#version 430 core

in vec2 fTexCoords;
out vec4 FragColor;

layout(binding = 0) uniform sampler2D frameTex;

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main(){
    vec2 tex_offset = 1.0 / textureSize(frameTex, 0) * 2; // gets size of single texel
    vec4 result = texture(frameTex, fTexCoords) * weight[0]; // current fragment's contribution

    for(int i = 1; i < 5; ++i)
    {
        result += texture(frameTex, fTexCoords + vec2(tex_offset.x * i, 0.0)) * weight[i];
        result += texture(frameTex, fTexCoords - vec2(tex_offset.x * i, 0.0)) * weight[i];
    }

    FragColor = result; //vec4(result, 1.0);
}