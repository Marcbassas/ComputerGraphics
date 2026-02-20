uniform mat4 u_model;
uniform mat4 u_viewprojection;

varying vec2 v_uv;

void main()
{
    v_uv = gl_MultiTexCoord0.xy;
    vec3 world_position = (u_model * vec4(gl_Vertex.xyz, 1.0)).xyz;
    gl_Position = u_viewprojection * vec4(world_position, 1.0);
}