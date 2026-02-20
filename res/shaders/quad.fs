#ifdef GL_ES
precision highp float;
#endif

uniform vec2 u_resolution;
uniform float u_time;
uniform int u_mode;  // 0..5 = a..f
uniform int u_task;  // 1=formules, 2=filtres, 3=transformacions
uniform sampler2D u_texture;

varying vec2 v_uv;

void main()
{
    vec2 uv = v_uv; //vector de coordenades UV del fragment actual: de (0,0) a (1,1) a través del quad
    float aspect = u_resolution.x / u_resolution.y; //aspect ratio de la pantalla
    vec2 p = uv * 2.0 - 1.0; //transforma las coordenadas UV de [0,1] a [-1,1] --> centrar el sistema de coordenades en el centre del quad
    p.x *= aspect; //adjusta la coordenada x per el aspect ratio per mantenir les proporcions correctes en pantalles no quadrades
    vec3 col = vec3(0.0); //inicializa el color del fragment a

    //2.2: FORMULES
    if (u_task == 1) 
    {
        if (u_mode == 0) // a)degradat horitzontal blau->vermell
        {
            col = mix(vec3(0.0,0.0,1.0), vec3(1.0,0.0,0.0), uv.x); //interpolació lineal entre blau i vermell segons la coordenada x de UV, creant un degradat horitzontal
        }
        else if (u_mode == 1) // b)gradient radial (vinyeta)
        {
            float d = length(p); //distància del fragment al centre del quad (0,0) en coordenades centrades
            col = vec3(1.0 - smoothstep(0.0, 1.4, d)); //aplica un degradat radial que va de blanc al centre a negre a les vores, utilitzant smoothstep per suavitzar la transició entre els colors segons la distància al centre
        }
        else if (u_mode == 2) //c) graella de línies
        {
            float fx = 8.0; float fy = 8.0; //frequencia de les línies en x i y, respectivament, controlant el nombre de línies que apareixen en cada direcció
            float vline = step(0.85, abs(sin(uv.x * 3.14159 * fx))); //línies verticals --> funció sinusoidal que oscil·la (-1 i 1) segons la coordenada x de UV multiplicada per pi i la frequencia fx, i aplicant step per crear línies nítides quan el valor absolut del sinus es major o igual a 0.85
            float hline = step(0.85, abs(sin(uv.y * 3.14159 * fy))); //línies horitzontals --> funció sinusoidal que oscil·la (-1 i 1)segons la coordenada y de UV multiplicada per pi y la frequencia fy, y apliucant step para crear línies nítides quan el valor absolut del sinus es major o igual a 0.85
            col = vec3(1.0,0.0,0.0)*vline + vec3(0.0,0.0,1.0)*hline; //combinacio de linies verticals i horitzontals 
        }
        else if (u_mode == 3) //d) degradat 2D (UV colors)
        {
            col = vec3(uv.x, uv.y, 0.0); //canal vermell = coord x de UV, canal verd = coord y de UV, canal blau = 0.0 --> creant degradat
        }
        else if (u_mode == 4) //e) escacs
        {
            vec2 c = floor(uv * 10.0); //calcula les coordenades de la cel·la d'escacs a la que pertany el fragment actual multiplicant les coordenades UV per 10 i aplicant floor per obtenir un valor enter que representa la cel·la
            col = vec3(mod(c.x + c.y, 2.0)); //aplica un patró d'escacs sumant les coordenades de la cel·la i aplicant mod 2 per obtenir un valor de 0.0 o 1.0 que alterna entre negre i blanc en les cel·les d'escacs
        }
        else // f) ona sinusoidal animada
        {
            float y = 0.5 + 0.25 * sin((uv.x * 6.28318) + u_time * 1.0); //y de la corba

            //màscara: 1 si uv.y < y (zona sota la corba)
            float m = 1.0 - step(y, uv.y);

            vec3 bg = vec3(0.0, 0.15, 0.0);
            vec3 fg = vec3(0.0, 0.8, 0.0);

            //vora suau (opcional)
            float edge = abs(uv.y - y);
            float soft = 1.0 - smoothstep(0.0, 0.02, edge);

            col = mix(bg, fg, m * soft + m * 0.4);
        }
    }

    //2.3: FILTRES D'IMATGE
    else if (u_task == 2) //aplicació de diferents filtres d'imatge segons el mode seleccionat
    {
        vec4 tex = texture2D(u_texture, uv); //mostra el color original de la textura a les coordenades UV del fragment actual
        vec3 texcol = tex.rgb; //color original de la textura sense alpha

        if (u_mode == 0) // a) original
        {
            col = texcol; //mostra el color original de la textura sense cap modificació
        }
        else if (u_mode == 1) // b) escala de grisos
        {
            float grey = dot(texcol, vec3(0.299, 0.587, 0.114)); //calcula el valor de grisos a partir del color original de la textura utilizando la fórmula de luminosidad
            col = vec3(grey); //mostra el color de grisos calculat
        }
        else if (u_mode == 2) // c) invertit (negatiu)
        {
            col = vec3(1.0) - texcol; //mostra el color invertit de la textura --> 1.0 menys el color original, invertint cada canal de color 
        }
        else if (u_mode == 3) // d) contrast/sepia
        {
            float grey = dot(texcol, vec3(0.299, 0.587, 0.114)); //calcula el valor de grisos a partir del color original de la textura --> fórmula de luminosidad, 
            col = vec3(grey * 1.2, grey * 0.9, grey * 0.6); //aplica un filtre de sepia al color de grisos, multiplicant cada canal de color per un factor diferent per crear l'efecte de sepia
        }
        else if (u_mode == 4) // e) threshold (blanc/negre)
        {
            float grey = dot(texcol, vec3(0.299, 0.587, 0.114)); //calcula el valor de grisos a partir del color original de la textura --> fórmula de luminosidad
            col = vec3(step(0.5, grey)); //aplica un filtre de threshold al color de grisos, mostrant blanc si el valor de grisos es major o igual
        }
        else // f) vinyeta + color shift
        {
            float vignette = 1.0 - smoothstep(0.4, 1.2, length(p));
            col = texcol * vignette;
            col.r = texture2D(u_texture, uv + vec2(0.005, 0.0)).r;
            col.b = texture2D(u_texture, uv - vec2(0.005, 0.0)).b;
        }
    }

    // ========== TASK 2.4: TRANSFORMACIONS ==========
    else if (u_task == 3)
    {
        if (u_mode == 0) //rotació animada
        {
            vec2 centered = uv - 0.5; //traslladem les coordenades UV al centre de la textura (0.5, 0.5) per facilitar la rotació
            float angle = u_time * 1.0; //angle de rotació que augmenta amb el temps, creant  animació de rotació contínua
            float ca = cos(angle); float sa = sin(angle); //calcul de cosinus i sinus de l'angle per aplicar la rotació
            vec2 rot = vec2(ca*centered.x - sa*centered.y, sa*centered.x + ca*centered.y); //aplicació de la rotació a les coordenades centrades --> obtenint les coordenades rotades
            vec2 rotUV = rot + 0.5; //traslladem les coordenades rotades de nou al rang [0,1] per poder mostrar la textura rotada
            //clamp --> evitar mostrar pixels de la textura quan rotUV surt de [0,1]
            vec2 clamped = clamp(rotUV, 0.0, 1.0); //mostra el color del pixel mes aprop de rotUV dins de la textura
            float inside = step(0.001, rotUV.x) * step(rotUV.x, 0.999) * step(0.001, rotUV.y) * step(rotUV.y, 0.999); //1.0 si rotUV dins de [0,1], 0.0 si rotUV fora de [0,1]
            col = texture2D(u_texture, clamped).rgb * inside; //mostra el color del pixel corresponent a rotUV si rotUV dins de [0,1], o negre si rotUV fora de [0,1]
        }
        else //pixelacio animada 
        {
            float pixels = floor(mix(4.0, 200.0, 0.5 + 0.5*sin(u_time * 0.8))); //de 4 a 200 pixels segons el temps
            vec2 pixUV = floor(uv * pixels) / pixels; //calcula coordenades UV del pixel corresponent a cada fragment
            col = texture2D(u_texture, pixUV).rgb; //mostra el color del pixel corresponent a cada fragment, creant l'efecte de pixelació
        }
    }

    gl_FragColor = vec4(col, 1.0); //alpha sempre a 1.0 (opacitat total)
}