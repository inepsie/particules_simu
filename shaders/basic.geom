#version 430
#define M_PI 3.1415926535897932384626433832795

layout(points) in;
layout(triangle_strip, max_vertices = 36) out;
uniform mat4 MVP;
uniform mat4 inv_view_mat;
out vec4 fColor;

void main(void) {
float size = 0.06;
//float size = 0.06;
int nb_slices = 16;
float slice_angle = (2 * M_PI) / nb_slices;

for(int i=0 ; i<=nb_slices ; ++i){
 gl_Position = MVP * (gl_in[0].gl_Position + (inv_view_mat * (size * vec4(cos(i*slice_angle), sin(i*slice_angle), 0.0, 0.0))));
 fColor = vec4(0.3, 0.5, 1.0, 1.0);
 EmitVertex();
 gl_Position = MVP * gl_in[0].gl_Position;
 fColor = vec4(0.3, 0.5, 1.0, 1.0);
 EmitVertex();
}
}
