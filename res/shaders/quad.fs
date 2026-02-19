#version 120

uniform sampler2D u_texture;
varying vec2 v_uv;

void main()
{
    gl_FragColor = vec4(v_uv, 0.0, 1.0);
}