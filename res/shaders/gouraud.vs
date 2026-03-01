#version 120

uniform mat4 u_model; //matriu de modelatge
uniform mat4 u_viewprojection; //matriu de vista i projecció combinades, que transforma les coordenades del món a les coordenades de la pantalla

uniform vec3 u_ambient_light; //color de la llum ambiental
uniform vec3 u_light_position; //posició de la llum en el món
uniform vec3 u_light_color; //color de la llum
uniform vec3 u_camera_position; //posició de la càmera en el món

uniform vec3 u_Ka; //coef ambient
uniform vec3 u_Kd; //coef difús
uniform vec3 u_Ks; //coef especular
uniform float u_shininess; //coef de brillantor 

varying vec2 v_uv; //coords de textura que es passen al fragment shader
varying vec3 v_color; // color calculat al vertex shader

void main(){
    v_uv = gl_MultiTexCoord0.xy; //passar les coordenades de textura al fragment shader

    //posicio i normal en world space
    vec3 world_position = (u_model * vec4(gl_Vertex.xyz, 1.0)).xyz; //transformar la posició del vèrtex al món utilitzant la matriu de modelatge
    vec3 world_normal = normalize((u_model * vec4(gl_Normal.xyz, 0.0)).xyz); //transformar la normal del vèrtex al món utilitzant la matriu de modelatge

    //VECTORS
    vec3 L = normalize(u_light_position - world_position); //vector que va des del vèrtex fins a la llum, normalitzat
    vec3 V = normalize(u_camera_position - world_position); //vector que va des del vèrtex fins a la càmera, normalitzat
    vec3 R = reflect(-L, world_normal); //vector de reflexió de la llum: calculat a partir del vector L i la normal del vèrtex, utilitzant la funció reflect que calcula el vector de reflexió a partir del vector d'incidència i la normal

    //AMBIENT
    vec3 ambient = u_Ka * u_ambient_light; //color ambient al vertex 

    //DIFUS
    float diff = max(dot(L, world_normal), 0.0); //el producte escalar entre el vector de llum i la normal del vèrtex determina la quantitat de llum difusa que incideix sobre el vèrtex i es multiplica pel coeficient difús i el color de la llum per obtenir el color difús al vèrtex
    vec3 diffuse = u_Kd * u_light_color * diff; //color difús al vertex

    //especular
    float spec = pow(max(dot(R, V), 0.0), u_shininess); //el producte escalar entre el vector de reflexió i el vector de vista determina la quantitat de llum especular que incideix sobre el vèrtex, elevat al coeficient de brillantor per controlar la mida del reflex especular i es multiplica pel coeficient especular i el color de la llum per obtenir el color especular al vèrtex
    vec3 specular = u_Ks * u_light_color * spec; //color especular al vertex

    v_color = ambient + diffuse + specular; //color final al vèrtex 

    gl_Position = u_viewprojection * vec4(world_position, 1.0); //gl_Position es la posició final del vèrtex a la pantalla
}