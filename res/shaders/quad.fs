#ifdef GL_ES
precision highp float;
#endif

uniform vec2 u_resolution;
uniform float u_time;
uniform int u_mode; // 0..5 => a..f

varying vec2 v_uv;

float rectChecker(vec2 p, float cells)
{
    // p in [0,1]
    vec2 c = floor(p * cells);
    float v = mod(c.x + c.y, 2.0);
    return v; // 0 or 1
}

void main()
{
    vec2 uv = v_uv;                 // [0,1]
    float aspect = u_resolution.x / u_resolution.y;

    // uv corregit per aspect per formes circulars sense deformació
    vec2 p = uv * 2.0 - 1.0;         // [-1,1]
    p.x *= aspect;

    vec3 col = vec3(0.0);

    // a) degradat blau->vermell horitzontal (com la mostra)
    if (u_mode == 0)
    {
        float t = uv.x;
        col = mix(vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0), t);
    }
    // b) “vinyeta”/radial en gris (fosc al centre o al revés)
    else if (u_mode == 1)
    {
        float d = length(p);                // 0 al centre
        float g = 1.0 - smoothstep(0.0, 1.2, d);
        col = vec3(g);
    }
    // c) graella de línies vermelles verticals + blaves horitzontals sobre fons negre
    else if (u_mode == 2)
    {
        // freqüències (pots canviar-les)
        float fx = 8.0;
        float fy = 8.0;

        // línies: fem servir abs(sin) i un threshold amb step
        float vx = abs(sin(uv.x * 3.14159 * fx));
        float hy = abs(sin(uv.y * 3.14159 * fy));

        float vline = step(0.85, vx); // 1 quan estàs a prop del pic (línia)
        float hline = step(0.85, hy);

        vec3 red = vec3(1.0, 0.0, 0.0) * vline;
        vec3 blue = vec3(0.0, 0.0, 1.0) * hline;

        col = red + blue; // si coincideixen suma (magenta)
    }
    // d) “heatmap” / quadriculat suau com el que tens ara (RGB = uv + foscor a baix-esquerra)
    else if (u_mode == 3)
    {
        // base: R=uv.x, G=uv.y, B=0
        vec3 base = vec3(uv.x, uv.y, 0.0);

        // enfosquir cap a baix-esquerra
        float shade = 1.0 - (1.0 - uv.x) * (1.0 - uv.y);
        col = base * shade;
    }
    // e) escacs blanc i negre
    else if (u_mode == 4)
    {
        float v = rectChecker(uv, 12.0);
        col = mix(vec3(0.0), vec3(1.0), v);
    }
    // f) “ona” verda sobre fons verd fosc (sense if: fem una màscara amb step)
    else
    {
        // y de la corba sinus
        float y = 0.5 + 0.25 * sin((uv.x * 6.28318) + u_time * 1.0);

        // màscara: 1 si uv.y < y (zona sota la corba)
        float m = 1.0 - step(y, uv.y);

        vec3 bg = vec3(0.0, 0.15, 0.0);
        vec3 fg = vec3(0.0, 0.8, 0.0);

        // vora suau (opcional)
        float edge = abs(uv.y - y);
        float soft = 1.0 - smoothstep(0.0, 0.02, edge);

        col = mix(bg, fg, m * soft + m * 0.4);
    }

    gl_FragColor = vec4(col, 1.0);
}