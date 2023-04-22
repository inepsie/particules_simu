#version 430

layout (location = 0) in vec4 vsiPosition;
layout (location = 1) in vec2 vsiTexCoord;

out vec2 vsoTexCoord;

void main(void) {
  gl_Position = vec4(vsiPosition.xy, 0.0, 1.0);
  vsoTexCoord = vsiTexCoord;
}
