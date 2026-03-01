#version 120

uniform vec3 u_ambient_light; //intensitat de la llum ambiental
uniform vec3 u_light_position; //posició de la llum en coordenades del món
uniform vec3 u_light_color; //color de la llum (pot incloure intensitat) en coordenades RGB
uniform vec3 u_camera_position; //posició de la càmera en coordenades del món

uniform vec3 u_Ka; //coef ambient
uniform vec3 u_Kd; //coef difús
uniform vec3 u_Ks; //coef especular
uniform float u_shininess; //coef de brillantor 

uniform sampler2D u_color_texture; //textura de color
uniform sampler2D u_specular_texture; //textura de especularitat
uniform sampler2D u_normal_texture; //textura de normals 
uniform int u_use_color_texture; //variable per activar o desactivar el us de la textura de color
uniform int u_use_specular_texture; //variable per activar o desactivar el us de la textura de especularitat
uniform int u_use_normal_texture; //variable per activar o desactivar el us de la textura de normals

varying vec2 v_uv; //coordenades de textura (UV) interpolades des del vertex shader
varying vec3 v_world_position; //posició del fragment en coordenades del món interpolada des del vertex shader
varying vec3 v_world_normal; //normal del fragment en coordenades del món interpolada des del vertex shader

void main(){
    //NORMAL
    vec3 N = normalize(v_world_normal); //normalitzem la normal interpolada del fragment

    if (u_use_normal_texture == 1) { //si la textura de normals esta activada --> obtenim la normal de la textura i la combinem amb la normal interpolada
        vec3 tex_normal = texture2D(u_normal_texture, v_uv).rgb;
        tex_normal = tex_normal * 2.0 - 1.0; // convertir [0,1] a [-1,1]
        N = normalize(mix(N, tex_normal, 0.5)); //combinar la normal interpolada (N) amb la normal de la textura (tex_normal) utilitzant una mescla lineal (mix) amb un factor de 0.5 (50% de cada una)
    }

    //VECTORS
    vec3 L = normalize(u_light_position - v_world_position); //vector de la posició del fragment a la posició de la llum (direcció de la llum)
    vec3 V = normalize(u_camera_position - v_world_position); //vector de la posició del fragment a la posició de la càmera (direcció de la vista)
    vec3 R = reflect(-L, N); //vector de reflexió de la llum (R) calculat a partir del vector de la llum (L) i la normal (N) utilitzant la funció reflect

    // Ka i Kd (substituïts per textura si activa)
    vec3 Ka = u_Ka;
    vec3 Kd = u_Kd;

    if (u_use_color_texture == 1) { //si la textura de color esta activada --> obtenim el color de la textura i l'utilitzem com a coeficients ambient i difús
        vec4 tex_color = texture2D(u_color_texture, v_uv); //obtenim el color de la textura en les coordenades UV del fragment
        Ka = tex_color.rgb; //utilitzem el color de la textura com a coeficient ambient (Ka)
        Kd = tex_color.rgb; //utilitzem el color de la textura com a coeficient difús (Kd)
    }

    // Ks (substituït per canal alpha de la textura si activa)
    vec3 Ks = u_Ks;
    if (u_use_specular_texture == 1) { //si la textura de especularitat esta activada --> obtenim el valor de especularitat de la textura (canal alpha) i l'utilitzem com a coeficient especular
        float spec_val = texture2D(u_color_texture, v_uv).a; //obtenim el valor de especularitat de la textura (canal alpha) en les coordenades UV del fragment
        Ks = vec3(spec_val); //utilitzem el valor de especularitat de la textura com a coeficient especular (Ks) convertit a un vector RGB (tots els components iguals al valor de especularitat)
    }

    //AMBIENT
    vec3 ambient = Ka * u_ambient_light;

    //DIFUS
    float diff = max(dot(L, N), 0.0);
    vec3 diffuse = Kd * u_light_color * diff;

    //ESPECULAR
    float spec = pow(max(dot(R, V), 0.0), u_shininess);
    vec3 specular = Ks * u_light_color * spec;

    vec3 color = ambient + diffuse + specular;

    gl_FragColor = vec4(color, 1.0); //color final del fragment --> suma de les components ambient, difusa i especular amb un alpha de 1.0 (opac)
}