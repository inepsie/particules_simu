#version 430

layout(local_size_x = 1000, local_size_y = 1, local_size_z = 1) in;

layout (std140, binding = 1) buffer current_pos_buff
{
 vec4 current_pos[];
};
layout (std140, binding = 2) buffer init_pos_buff
{
 vec4 init_pos[];
};
layout (std140, binding = 3) buffer vitesse_buff
{
 vec4 vitesse[];
};

uniform float dt;

void main()
{
 uint gid = gl_GlobalInvocationID.x;
 vec3 new_pos = current_pos[gid].xyz;
 vec3 initp = init_pos[gid].xyz;
 vec3 vit = vitesse[gid].xyz;

 new_pos += 0.01 * vit;

 /*
 if(new_pos.x > 15.0f ||
    new_pos.x < -15.0f ||
    new_pos.y > 15.0f ||
    new_pos.y < -15.0f ||
    new_pos.z > 15.0f ||
    new_pos.z < -15.0f){
  new_pos = initp;
 }
 */
 if(length(new_pos) > 100.0f){
  new_pos = initp;
 }
 current_pos[gid].xyz = new_pos;
}
