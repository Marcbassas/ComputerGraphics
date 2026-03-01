#version 120

varying vec2 v_uv; //coordenades de textura (UV)
varying vec3 v_color; //color interpolada (Gouraud)

uniform sampler2D u_color_texture; //textura de color
uniform int u_use_color_texture; //variable per activar o desactivar el us de la textura de color

void main(){
    vec3 col = v_color; //color interpolada (Gouraud)

    if (u_use_color_texture == 1) //si la textura de color esta activada --> multipliquem el color interpolado per el color de la textura
        col *= texture2D(u_color_texture, v_uv).rgb; //el col = color interpolada (Gouraud) * color de la textura (en les coordenades UV)

    gl_FragColor = vec4(col, 1.0); //color final del fragment --> color calculado (col) amb un alpha de 1.0 (opac)
}