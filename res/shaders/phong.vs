#version 120

uniform mat4 u_model; //matriu de modelatge
uniform mat4 u_viewprojection; //matriu de vista i projecció combinades --> transforma les coordenades del món a les coordenades de la pantalla

varying vec2 v_uv; //coords de textura que es passen al fragment shader
varying vec3 v_world_position; //posició del vèrtex en el món --> calculada al vertex shader i passada al fragment shader per al càlcul de la il·luminació per píxel
varying vec3 v_world_normal; //normal del vèrtex en el món --> calculada al vertex shader i passada al fragment shader per al càlcul de la il·luminació per píxel

void main(){
    v_uv = gl_MultiTexCoord0.xy; //passar les coordenades de textura al fragment shader
     
    v_world_position = (u_model * vec4(gl_Vertex.xyz, 1.0)).xyz; //transformar la posició del vèrtex al món utilitzant la matriu de modelatge i passar-la al fragment shader per al càlcul de la il·luminació per píxel
    v_world_normal = (u_model * vec4(gl_Normal.xyz, 0.0)).xyz; //transformar la normal del vèrtex al món utilitzant la matriu de modelatge i passar-la al fragment shader per al càlcul de la il·luminació per píxel

    gl_Position = u_viewprojection * vec4(v_world_position, 1.0); //gl_Position es la posició final del vèrtex a la pantalla, calculada transformant la posició del vèrtex al món utilitzant la matriu de vista i projecció combinades
}