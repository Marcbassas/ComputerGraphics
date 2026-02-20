#include "application.h"
#include "mesh.h"
#include "shader.h"
#include "utils.h" 

Application::Application(const char* caption, int width, int height) { //constructor de la classe application, crea la finestra i inicialitza les variables
	this->window = createWindow(caption, width, height);

	int w,h; 
	SDL_GetWindowSize(window,&w,&h);

	this->mouse_state = 0; //inicialitzar mouse state = 0 (cap botó premut)
	this->time = 0.f; //inicialitzar temps = 0
	this->window_width = w; //inicialitzar amplada finestra
	this->window_height = h; //inicialitzar altura finestra
	this->keystate = SDL_GetKeyboardState(nullptr); //obtenir estat teclat 

	this->framebuffer.Resize(w, h);//inicialitzar framebuffer amb dimensions de la finestra

	// initialize last mouse positions
	int mx, my;
	SDL_GetMouseState(&mx, &my);
	last_mouse_x = mx;
	last_mouse_y = my;
}

Application::~Application(){ 
    delete quad_mesh;
    delete quad_shader;
    current_mode = 0;
    quad_mode = 0;
}

void Application::Init(void) { //inicialitza l'aplicació
    std::cout << "Initiating app..." << std::endl;

    //INICIALITZACIO DE LA MALLA, ENTITATS I CÀMERA
    Mesh* m = new Mesh();
    m->LoadOBJ("meshes/lee.obj");

    //crear les 3 entitats amb la malla carregada i posar-les a diferents posicions
    for (int i = 0; i < 3; i++) {
        Entity* ent = new Entity(m);

        // CARREGAR TEXTURA PER A AQUESTA ENTITAT 
        ent->texture = new Image();
        ent->texture->LoadTGA("textures/lee_color_specular.tga", true);

        //inicialitzar la matriu de model per a cada entitat
        Matrix44 m_trans; //matriu de transformació per a cada entitat
        m_trans.MakeTranslationMatrix((i - 1) * 6.0f, 0.0f, 0.0f); //posicionar entitats 

        Matrix44 m_rot; //matriu de rotació per a cada entitat
        m_rot.MakeRotationMatrix(3.14159f, Vector3(1, 0, 0)); // Rotació 180° en X per posar-lo dret

        Matrix44 m_scale; //matriu d'escalat per a cada entitat
        float scale_factor = 5.0f; //factor de escala per fer la malla més gran
        m_scale.MakeScaleMatrix(scale_factor, scale_factor, scale_factor);//escalat de la malla per fer-la més gran

        //Ordre: Model = Translation * Rotation * Scale (T * R * S)
        ent->model = m_trans * m_rot * m_scale;

        ent->time = i * 1.0f;
        entities.push_back(ent);
    }

    camera = new Camera();

    //POSICIÓ INICIAL CAMERA
    camera->LookAt(
        Vector3(0, -1, 8),    //posició de la càmera (eye)
        Vector3(0, -1, 0),     //punt al que mira la càmera (center)
        Vector3(0, 1, 0)       //vector up de la càmera (up)
    );

    //configuració de perspectiva 
    camera->SetPerspective(
        45.0f,                              //FOV
        window_width / (float)window_height, //aspect ratio
        0.1f,                               //Near plane 
        200.0f                              //Far plane 
    );

    quad_mesh = new Mesh();
    quad_mesh->CreateQuad();
    quad_shader = new Shader();
    bool ok = quad_shader->Load("shaders/quad.vs", "shaders/quad.fs");

    std::cout << "Shader Load ok? " << ok << std::endl;
    if (!ok || quad_shader->HasInfoLog())
        std::cout << quad_shader->GetInfoLog() << std::endl;
        
    // Carrega textura OpenGL pel quad
    quad_texture = Texture::Get("textures/lee_color_specular.tga");
}

void Application::Render(void)
{
    // Camera setup per 3D
    camera->SetPerspective(camera->fov, window_width / (float)window_height, camera->near_plane, camera->far_plane);
    camera->LookAt(camera->eye, camera->center, camera->up);

    // MODE 0: UNA ENTITAT (3D)
    if (current_mode == 0)
    {
        framebuffer.Fill(Color::BLACK);

        FloatImage zbuffer(framebuffer.width, framebuffer.height);
        zbuffer.Fill(999999.0f);

        if (entities.size() >= 2)
            entities[1]->Render(&framebuffer, camera, use_occlusions ? &zbuffer : nullptr);

        framebuffer.Render();
        return;
    }

    // MODE 1: QUAD GLSL (a..f)
    if (current_mode == 1)
    {
        glViewport(0, 0, window_width, window_height);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        quad_shader->Enable();

        // uniforms
        quad_shader->SetInt("u_mode", quad_mode); // 0..5 (a..f)
        quad_shader->SetVector2("u_resolution", Vector2((float)window_width, (float)window_height));
        quad_shader->SetFloat("u_time", time);

        quad_mesh->Render(GL_TRIANGLES);

        quad_shader->Disable();
        return;
    }

    // MODES 2,3,4: (de moment, no fan res -> per no liar, mostra negre)
    if (current_mode >= 2 && current_mode <= 4)
    {
        framebuffer.Fill(Color::BLACK);
        framebuffer.Render();
        return;
    }

    // MODE 5: VARIES ENTITATS (3D)
    if (current_mode == 5)
    {
        FloatImage zbuffer(framebuffer.width, framebuffer.height);
        zbuffer.Fill(999999.0f);

        framebuffer.Fill(Color::BLACK);

        for (size_t i = 0; i < entities.size(); ++i)
            entities[i]->Render(&framebuffer, camera, use_occlusions ? &zbuffer : nullptr);

        framebuffer.Render();
        return;
    }
}


//actualittza l'aplicacio en funcio del temps que ha passat
void Application::Update(float seconds_elapsed){
	this->time += seconds_elapsed; //incrementar temps global amb el temps transcorregut des de l'última actualització

	if (current_mode == 2) { //si estem en mode d'animació
		//LAB1: actualitzar la pluja
		//rain.Update(seconds_elapsed, window_width, window_height); //update(temps, amplada finestra, altura finestra)
		
        //LAB2: Actualitzar les entitats 3D 
		for (int i = 0; i < entities.size(); ++i) { //per cada entitat
			entities[i]->Update(seconds_elapsed); //actualitzar l'entitat amb el temps transcorregut des de l'última actualització per animar les entitats
		}
	}
}



void Application::OnKeyPressed(SDL_KeyboardEvent event) { //tecla premuda
    // KEY CODES: https://wiki.libsdl.org/SDL2/SDL_Keycode

    switch (event.keysym.sym) {

	case SDLK_ESCAPE: // ESC: sortir de aplicació
        exit(0);
        break;

        // 0: una entitat (3D)
    case SDLK_0:
        current_mode = 0;
        std::cout << "Mode 0: Single entity (3D)\n";
        break;

        // 1: QUAD (GLSL) + submodes a..f
    case SDLK_1:
        current_mode = 1;
        std::cout << "Mode 1: Quad GLSL (press a..f)\n";
        break;

        // 2,3,4 lliures
    case SDLK_2:
        current_mode = 2;
        std::cout << "Mode 2: free\n";
        break;

    case SDLK_3:
        current_mode = 3;
        std::cout << "Mode 3: free\n";
        break;

    case SDLK_4:
        current_mode = 4;
        std::cout << "Mode 4: free\n";
        break;

        // 5: varies entitats (3D)
    case SDLK_5:
        current_mode = 5;
        std::cout << "Mode 5: Multiple entities (3D)\n";
        break;

        // Submodes només quan estàs al mode 1 (quad)
    case SDLK_a:
        if (current_mode == 1) { quad_mode = 0; std::cout << "Quad submode: a\n"; }
        break;
    case SDLK_b:
        if (current_mode == 1) { quad_mode = 1; std::cout << "Quad submode: b\n"; }
        break;
    case SDLK_c:
        if (current_mode == 1) { quad_mode = 2; std::cout << "Quad submode: c\n"; }
        break;
    case SDLK_d:
        if (current_mode == 1) { quad_mode = 3; std::cout << "Quad submode: d\n"; }
        break;
    case SDLK_e:
        if (current_mode == 1) { quad_mode = 4; std::cout << "Quad submode: e\n"; }
        break;
    case SDLK_f:
        if (current_mode == 1) { quad_mode = 5; std::cout << "Quad submode: f\n"; }
        break;

	case SDLK_n: //N = near 
        current_camera_property = CAM_NEAR;
        std::cout << "Camera property: NEAR" << std::endl;
        break;

	case SDLK_g: //F = far
        current_camera_property = CAM_FAR;
        std::cout << "Camera property: FAR" << std::endl;
        break;

	case SDLK_v: //FOV = camp de visió
        current_camera_property = CAM_FOV;
        std::cout << "Camera property: FOV" << std::endl;
        break;

    // Render toggles
    case SDLK_t: // T: toggle texture vs vertex color
    {
        for (auto e : entities) e->use_texture = !e->use_texture;
        std::cout << "Toggled use_texture for all entities: " << (entities.size() ? entities[0]->use_texture : false) << std::endl;
        break;
    }

    case SDLK_z: // Z: toggle occlusions
    {
        use_occlusions = !use_occlusions;
        std::cout << "Toggled occlusions: " << use_occlusions << std::endl;
        break;
    }

    case SDLK_u:
    {
        for (auto e : entities) e->interpolate_uvs = !e->interpolate_uvs;
        std::cout << "Toggled interpolate_uvs for all entities: " << (entities.size() ? entities[0]->interpolate_uvs : false) << std::endl;
        break;
    }

    case SDLK_w: // W: toggle wireframe / filled
    {
        for (auto e : entities) {
            if (e->mode == Entity::eRenderMode::WIREFRAME) e->mode = Entity::eRenderMode::TRIANGLES_INTERPOLATED;
            else e->mode = Entity::eRenderMode::WIREFRAME;
        }
        std::cout << "Toggled wireframe/triangles for all entities" << std::endl;
        break;
    }

    //AUGMENTAR 
    case SDLK_PLUS: // +
	case SDLK_EQUALS: //tecla + sense shift
	case SDLK_KP_PLUS: //tecla + del teclaT numéric
    {
		if (current_camera_property == CAM_NONE) //si no s'ha seleccionat cap propietat de la càmera, no fer res
            break;

        switch (current_camera_property) {

		case CAM_NEAR: //augmentar el planol de prop de la càmera
            camera->near_plane += 0.01f * camera->near_plane + 0.01f;
            if (camera->near_plane < 0.001f)
                camera->near_plane = 0.001f;
            if (camera->near_plane > camera->far_plane - 0.01f)
                camera->near_plane = camera->far_plane - 0.01f;
			std::cout << "Camera near: " << camera->near_plane << std::endl; //missatge del nou planol de prop (consola)
            break;

		case CAM_FAR: //augmentar el planol de lluny de la càmera
            camera->far_plane += 10.0f;
			std::cout << "Camera far: " << camera->far_plane << std::endl; //missatge del nou planol de lluny (consola)
            break;

		case CAM_FOV: //augmentar el camp de visió de la càmera
            camera->fov += 1.0f;
            if (camera->fov > 179.0f)
                camera->fov = 179.0f;
			std::cout << "Camera fov: " << camera->fov << std::endl; //missatge nova FOV (consola)
            break;

        default:
            break;
        }

		camera->UpdateProjectionMatrix(); //actualitzar la matriu de projecció de la càmera amb les noves propietats
        break;
    }

    //DISMINUIR 
	case SDLK_MINUS: // -
	case SDLK_KP_MINUS: //tecla - del teclat numéric
    {
		if (current_camera_property == CAM_NONE) //si no s'ha seleccionat cap propietat de la càmera no fer res
            break;

        switch (current_camera_property) {

		case CAM_NEAR: //disminuir el planol de prop de la càmera
            camera->near_plane -= 0.01f * camera->near_plane + 0.01f;
            if (camera->near_plane < 0.001f)
                camera->near_plane = 0.001f;
            if (camera->near_plane > camera->far_plane - 0.01f)
                camera->near_plane = camera->far_plane - 0.01f;
			std::cout << "Camera near: " << camera->near_plane << std::endl; //missatge del nou planol de prop
            break;

		case CAM_FAR: //disminuir el planol de lluny de la càmera
            camera->far_plane -= 10.0f;
            if (camera->far_plane < camera->near_plane + 1.0f)
                camera->far_plane = camera->near_plane + 1.0f;
			std::cout << "Camera far: " << camera->far_plane << std::endl; //missatge del nou planol de lluny
            break;

		case CAM_FOV: //disminuir el camp de visió de la càmera
            camera->fov -= 1.0f;
            if (camera->fov < 1.0f)
                camera->fov = 1.0f;
			std::cout << "Camera fov: " << camera->fov << std::endl; //missatge de la nova FOV
            break;

        default:
            break;
        }

		camera->UpdateProjectionMatrix(); //actualitzar la matriu de projecció de la càmera amb les noves propietats
        break;
    }
    }
}


void Application::OnMouseMove(SDL_MouseButtonEvent event) { //MOVIMENT DEL RATOLI
    int x = event.x;
    int y = event.y;

    int xrel = x - last_mouse_x;
    int yrel = y - last_mouse_y;

    last_mouse_x = x;
    last_mouse_y = y;

    if (current_mode >= 1 && current_mode <= 4) return; //si estem en mode (1.2.3.4) no moure la càmera

    int buttons = SDL_GetMouseState(NULL, NULL); //estat actual dels botons del ratolí (quins estan premuts)

    //orbitar al voltant del centre amb el botó esquerre
    if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        float sensitivity = 0.005f; //sens del moviment del ratolí a la rotació de la càmera
        float angY = -xrel * sensitivity; //invertir la rotacio horitzontal per: moure el ratolí a la dreta giri la càmera a la dreta
        float angX = yrel * sensitivity; //invertir la rotació vertical per: moure el ratolí cap avall giri la càmera cap avall

        Vector3 dir = camera->eye - camera->center; //vector de direcció des del centre cap a l'eye (direcció de la càmera)

        //rotar al voltant de l'eix up de la càmera (rotació horizontal)
        Matrix44 rotY; rotY.MakeRotationMatrix(angY, camera->up);
        dir = rotY.RotateVector(dir);

        //rotar al voltant de l'eix perpendicular a la direcció i el vector up (rotació vertical)
        Vector3 right = camera->up.Cross(dir).Normalize();
        Matrix44 rotX; rotX.MakeRotationMatrix(angX, right);
        dir = rotX.RotateVector(dir);

        camera->eye = camera->center + dir; //actualitzar la posició de l'eye mantenint la mateixa distància al centre per orbitar al voltant del centre
    }
    //PAN amb el botó dret
    else if (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        float panSpeed = 0.01f; //velocitat de pan = sensibilitat del pan al moviment del ratolí
        Vector3 forward = (camera->center - camera->eye).Normalize(); //vector de direcció de la càmera des de l'eye cap al center (normalitzat)
        Vector3 right = forward.Cross(camera->up).Normalize(); //vector perpendicular a la direcció de la cámara y el vector up --> per PAN horizontal

        //moure el eye i el center en la direcció oposada al moviment del ratolí per aconseguir un efecte de pan
        camera->eye += (-xrel * panSpeed) * right + (-yrel * panSpeed) * camera->up;
        camera->eye += (-xrel * panSpeed) * right + (-yrel * panSpeed) * camera->up;
        camera->center += (-xrel * panSpeed) * right + (-yrel * panSpeed) * camera->up;
    }
}


void Application::OnMouseButtonDown(SDL_MouseButtonEvent event) { //CLICK DEL RATOLI (PRESS)
	//actualitzar la posició del ratolí i el seu estat
    last_mouse_x = event.x;
    last_mouse_y = event.y;
}


void Application::OnMouseButtonUp(SDL_MouseButtonEvent event) { //CLICK DEL RATOLI (RELEASE)
}



void Application::OnWheel(SDL_MouseWheelEvent event) //ZOOM AMB LA RODA DEL RATOLI
{
	float dy = event.preciseY;

	//zoom amb la roda del ratolí (modes 1 i 2)
	if (current_mode != 0)
	{
		float zoom_speed = 1.0f; // Velocitat de zoom
		Vector3 direction = (camera->center - camera->eye).Normalize(); //direcció de la càmera des de l'eye cap al center (normalitzada)
		float current_distance = camera->eye.Distance(camera->center); //distància actual de la càmera al centre (punt al que mira)
		float new_distance = current_distance - (dy * zoom_speed); 

		//limitar distancia min i max per evitar que la càmera es posi massa aprop o massa lluny del centre
		if (new_distance < 2.0f) new_distance = 2.0f;     //no més aprop de 2 unitats
		if (new_distance > 50.0f) new_distance = 50.0f;   //no més lluny de 50 unitats

		//actualitzar la posició de l'eye mantenint la direcció
		camera->eye = camera->center - (direction * new_distance);
	}
}

void Application::OnFileChanged(const char* filename)//detecta canvi en un fitxer
{ 
	Shader::ReloadSingleShader(filename);
}