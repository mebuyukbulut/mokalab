#version 430 core

layout(location = 0) out uint outObjectID;

uniform uint objectID;

void main()
{
    outObjectID = objectID;
}


// out vec4 FragColor;
// uniform int objectID;


// void main(){
//     //int a = 120;
//     FragColor = vec4(
//         float((objectID & 0x000000FF)) / 255.0,
//         float((objectID & 0x0000FF00) >> 8) / 255.0,
//         float((objectID & 0x00FF0000) >> 16) / 255.0,
//         1.0
//     );
//     //FragColor = vec4(1,1,1,1.0);

// }	