#version 430

layout (location = 0) in vec4 vsiPosition;

uniform mat4 MVP;
uniform mat4 inv_view_mat;

void main(void) {
 gl_Position = vec4(vsiPosition.xyz, 1.0);
}
