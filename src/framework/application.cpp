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
	//LEE OBJ
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
        m_rot.MakeRotationMatrix(0.0f, Vector3(1, 0, 0)); // Rotació 180° en X per posar-lo dret

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
        Vector3(0, 2, 5),    //posició de la càmera (eye)
        Vector3(0, 1, 0),     //punt al que mira la càmera (center)
        Vector3(0, 1, 0)       //vector up de la càmera (up)
    );

    //configuració de perspectiva 
    camera->SetPerspective(
        45.0f,                              //FOV
        window_width / (float)window_height, //aspect ratio
        0.1f,                               //Near plane 
        200.0f                              //Far plane 
    );

    //LAB4 QUADS
    quad_mesh = new Mesh();
    quad_mesh->CreateQuad();
    quad_shader = new Shader();
    bool ok = quad_shader->Load("shaders/quad.vs", "shaders/quad.fs");

    std::cout << "Shader Load ok? " << ok << std::endl;
    if (!ok || quad_shader->HasInfoLog())
        std::cout << quad_shader->GetInfoLog() << std::endl;
        
    // Carrega textura OpenGL pel quad
    quad_texture = Texture::Get("images/fruits.png");

    //LAB4 2.5 --> CREAR SHADER PER A RASTERITZACIÓ
    raster_shader = new Shader();
    raster_shader->Load("shaders/raster.vs", "shaders/raster.fs");

    for (auto ent : entities) { //per cada entitat, assignar el shader de rasterització i la textura corresponent
        ent->shader = raster_shader;
        ent->texture_gpu = Texture::Get("textures/lee_color_specular.tga");
    }

	//LAB5 - CONFIGURACIO DE LLUMS I MATERIALS
	uniform_data.ambient_light = Vector3(0.1f, 0.1f, 0.1f); //llum ambient global 
	uniform_data.lights[0].position = Vector3(5.0f, 10.0f, 5.0f); //posició de la llum 0, llum direccional des de dalt i a la dreta
	uniform_data.lights[0].color = Vector3(1.0f, 1.0f, 1.0f); //color de la llum 0 (blanca)
	uniform_data.num_lights = 1; //nombre de llunms actives

    //segona llum
    uniform_data.lights[1].position = Vector3(-5.0f, 5.0f, 5.0f);
    uniform_data.lights[1].color = Vector3(0.0f, 0.5f, 1.0f); // blava


    //crear material Gouraud
	gouraud_material = new Material(); //material per a renderitzat Gouraud
	gouraud_material->shader = new Shader(); //shader per a renderitzat Gouraud
	gouraud_material->shader->Load("shaders/gouraud.vs", "shaders/gouraud.fs"); //carregar shader de Gouraud 
	gouraud_material->color_texture = Texture::Get("textures/lee_color_specular.tga"); //assignar textura de color al material Gouraud
	gouraud_material->use_color_texture = true; //activar el us de la textura de color al material Gouraud

    //crear material Phong
	phong_material = new Material(); //material per a renderitzat Phong
	phong_material->shader = new Shader(); //shader per a renderitzat Phong
	phong_material->shader->Load("shaders/phong.vs", "shaders/phong.fs"); //carregar shader de Phong
	phong_material->color_texture = Texture::Get("textures/lee_color_specular.tga"); //assignar textura de color al material Phong
	phong_material->use_color_texture = true; //activar el us de la textura de color al material Phong

    //assignar material Gouraud a les entitats per defecte
	for (auto ent : entities) //per cada entitat = assignar el material de Gouraud
        ent->material = gouraud_material; 

	// Material Gouraud --> assignar les textures corresponents al material de Gouraud
    gouraud_material->color_texture = Texture::Get("textures/lee_color_specular.tga");
    gouraud_material->specular_texture = Texture::Get("textures/lee_color_specular.tga"); // canal alpha
    gouraud_material->normal_texture = Texture::Get("textures/lee_normal.tga");

	// Material Phong --> assignar les textures corresponents al material de Phong
    phong_material->color_texture = Texture::Get("textures/lee_color_specular.tga");
    phong_material->specular_texture = Texture::Get("textures/lee_color_specular.tga"); // canal alpha
    phong_material->normal_texture = Texture::Get("textures/lee_normal.tga");
}

void Application::Render(void){
    //camera setup per 3D
    camera->SetPerspective(camera->fov, window_width / (float)window_height, camera->near_plane, camera->far_plane);
    camera->LookAt(camera->eye, camera->center, camera->up);

    //MODE 1: QUAD GLSL (a..f)
    if (current_mode == 1)
    {
        glViewport(0, 0, window_width, window_height);

		glDisable(GL_DEPTH_TEST); //desactivar test de profunditat per renderitzat 2D
		glDisable(GL_CULL_FACE); //desactivar cull face per renderitzat 2D
         
		glClearColor(0.f, 0.f, 0.f, 1.f); //color de fons negre
		glClear(GL_COLOR_BUFFER_BIT); //netejar el framebuffer amb el color de fons

		glMatrixMode(GL_PROJECTION); //configurar matriu de projecció ortogràfica per a renderitzat 2D
		glLoadIdentity(); //carregar matriu identitat a la matriu de projecció
		glMatrixMode(GL_MODELVIEW); //configurar matriu de model per a renderitzat 2D
		glLoadIdentity(); //carregar matriu identitat a la matriu de model

		quad_shader->Enable(); //activar el shader per renderitzar el quad

        //uniforms
        quad_shader->SetInt("u_mode", quad_mode); 
        quad_shader->SetInt("u_task", current_task_num);
		quad_shader->SetVector2("u_resolution", Vector2((float)window_width, (float)window_height));
        quad_shader->SetFloat("u_time", time);
		//passar la textura al shader (si existeix)
        if (quad_texture) quad_shader->SetTexture("u_texture", quad_texture);
        
		quad_mesh->Render(GL_TRIANGLES); //renderitzar el quad amb el shader activat, el shader farà diferents coses segons el submode i la tasca actuals

		quad_shader->Disable(); //desactivar el shader després de renderitzar el quad
        return;
    }

    //MODE 2 I 3 = IMATGE DE FRUITA
    if (current_mode == 2 || current_mode == 3)
    {
		framebuffer.Fill(Color::BLACK); //netejar framebuffer amb negre
		framebuffer.Render(); //renderitzar el framebuffer a la pantalla (2.2)
        return;
    }

    //MODE 4: RENDERITZAT 3D GPU (Task 2.5)
    if (current_mode == 4){
		glEnable(GL_DEPTH_TEST); //activar test de profunditat per renderitzat 3D
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f); //color de fons gris fosc
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //netejar color i profunditat

        camera->UpdateViewMatrix(); 
        camera->UpdateProjectionMatrix();
        camera->UpdateViewProjectionMatrix();

        if (in_lab4){
            //LAB4: shader simple
			if (entities.size() >= 2) //si hi ha almenys 2 entitats --> renderitzar la segona entitat passant-li la càmera per a que pugui calcular les matrius de transformació i renderitzar-se amb el shader simple
                entities[1]->Render(camera);
        }
        else{
            //LAB5: material amb llums
			uniform_data.viewprojection = camera->viewprojection_matrix; //passar la matriu de vista-projecció combinada al shader a través de la estructura uniform_data
			uniform_data.camera_position = camera->eye; //passar la posició de la càmera al shader a través de la estructura uniform_data
			if (entities.size() >= 2) //si hi ha almenys 2 entitats --> renderitzar la segona entitat amb les dades uniformes globals
                entities[1]->Render(uniform_data);
        }
        return;
    }

    //MODE 5: VARIES ENTITATS (3D GPU)
    if (current_mode == 5)
    {
		glEnable(GL_DEPTH_TEST); //activar test de profunditat per renderitzat 3D
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f); //color de fons gris fosc
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //netejar color i profunditat

        camera->UpdateViewMatrix();
        camera->UpdateProjectionMatrix();
        camera->UpdateViewProjectionMatrix();

		for (auto e : entities) //per cada entitat, renderitzar-la passant-li la càmera per a que pugui calcular les matrius de transformació i renderitzar-se amb el shader associat a l'entitat
            e->Render(camera);

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



void Application::OnKeyPressed(SDL_KeyboardEvent event) {
    switch (event.keysym.sym) {

    case SDLK_ESCAPE:
        exit(0);
        break;

        // ---- TECLES LAB4 ----
    case SDLK_1:
        if (in_lab4) { current_mode = 1; current_task_num = 1; }
        else { uniform_data.num_lights = 1; std::cout << "Lights: 1\n"; }
        break;

    case SDLK_2:
        if (in_lab4) { current_mode = 1; current_task_num = 2; }
        else { uniform_data.num_lights = 2; std::cout << "Lights: 2\n"; }
        break;

    case SDLK_3:
        if (in_lab4) { current_mode = 1; current_task_num = 3; }
        break;

    case SDLK_4:
        current_mode = 4;
        break;

    case SDLK_5:
        current_mode = 5;
        break;

        // ---- SUBMODES QUAD (a..f) ----
    case SDLK_a:
        if (current_mode == 1) { quad_mode = 0; std::cout << "Quad submode: a\n"; }
        break;
    case SDLK_b:
        if (current_mode == 1) { quad_mode = 1; std::cout << "Quad submode: b\n"; }
        break;
    case SDLK_c:
        if (current_mode == 1) { quad_mode = 2; std::cout << "Quad submode: c\n"; }
        else if (!in_lab4 && gouraud_material && phong_material) {
            gouraud_material->use_color_texture = !gouraud_material->use_color_texture;
            phong_material->use_color_texture = !phong_material->use_color_texture;
            std::cout << "Color texture: " << gouraud_material->use_color_texture << "\n";
        }
        break;
    case SDLK_d://submode quad
        if (current_mode == 1) { quad_mode = 3; std::cout << "Quad submode: d\n"; }
        break;
	case SDLK_e: //submode quad
        if (current_mode == 1) { quad_mode = 4; std::cout << "Quad submode: e\n"; }
        break;
	case SDLK_f: //submode quad
        if (current_mode == 1) { quad_mode = 5; std::cout << "Quad submode: f\n"; }
        break;

        // ---- TECLES LAB5 ----
    case SDLK_g: // G: Gouraud
        if (!in_lab4 && gouraud_material) {
            use_gouraud = true;
            for (auto e : entities) e->material = gouraud_material;
            std::cout << "Shading: GOURAUD\n";
        }
        break;

    case SDLK_p: // P: Phong
        if (!in_lab4 && phong_material) {
            use_gouraud = false;
            for (auto e : entities) e->material = phong_material;
            std::cout << "Shading: PHONG\n";
        }
        break;

    case SDLK_s: // S: toggle specular texture
        if (!in_lab4 && gouraud_material && phong_material) {
            gouraud_material->use_specular_texture = !gouraud_material->use_specular_texture;
            phong_material->use_specular_texture = !phong_material->use_specular_texture;
            std::cout << "Specular texture: " << gouraud_material->use_specular_texture << "\n";
        }
        break;

    case SDLK_n: // N: toggle normal texture (LAB5) o near plane (LAB4)
        if (!in_lab4 && gouraud_material && phong_material) {
            gouraud_material->use_normal_texture = !gouraud_material->use_normal_texture;
            phong_material->use_normal_texture = !phong_material->use_normal_texture;
            std::cout << "Normal texture: " << gouraud_material->use_normal_texture << "\n";
        }
		else { //LAB4: seleccionar prop de càmera NEAR
            current_camera_property = CAM_NEAR;
            std::cout << "Camera property: NEAR\n";
        }
        break;

        // ---- TOGGLE LAB4/LAB5 ----
    case SDLK_l:
        in_lab4 = !in_lab4;
        std::cout << "Now in " << (in_lab4 ? "LAB4" : "LAB5") << std::endl;
        break;

    case SDLK_v:
        current_camera_property = CAM_FOV;
        std::cout << "Camera property: FOV\n";
        break;

    case SDLK_t:
        for (auto e : entities) e->use_texture = !e->use_texture;
        break;

	case SDLK_z: // toggle occlusions (LAB4)
        use_occlusions = !use_occlusions;
        break;

	case SDLK_u: // toggle UV interpolation (LAB4)
        for (auto e : entities) e->interpolate_uvs = !e->interpolate_uvs;
        break;

    case SDLK_w:
        for (auto e : entities) {
            if (e->mode == Entity::eRenderMode::WIREFRAME)
                e->mode = Entity::eRenderMode::TRIANGLES_INTERPOLATED;
            else
                e->mode = Entity::eRenderMode::WIREFRAME;
        }
        break;

        // ---- AUGMENTAR / DISMINUIR ----
    case SDLK_PLUS:
    case SDLK_EQUALS:
    case SDLK_KP_PLUS:
    {
        if (current_camera_property == CAM_NONE) break;
        switch (current_camera_property) {
        case CAM_NEAR:
            camera->near_plane += 0.01f * camera->near_plane + 0.01f;
            if (camera->near_plane < 0.001f) camera->near_plane = 0.001f;
            if (camera->near_plane > camera->far_plane - 0.01f) camera->near_plane = camera->far_plane - 0.01f;
            std::cout << "Camera near: " << camera->near_plane << std::endl;
            break;
        case CAM_FAR:
            camera->far_plane += 10.0f;
            std::cout << "Camera far: " << camera->far_plane << std::endl;
            break;
        case CAM_FOV:
            camera->fov += 1.0f;
            if (camera->fov > 179.0f) camera->fov = 179.0f;
            std::cout << "Camera fov: " << camera->fov << std::endl;
            break;
        default: break;
        }
        camera->UpdateProjectionMatrix();
        break;
    }

    case SDLK_MINUS:
    case SDLK_KP_MINUS:
    {
        if (current_camera_property == CAM_NONE) break;
        switch (current_camera_property) {
        case CAM_NEAR:
            camera->near_plane -= 0.01f * camera->near_plane + 0.01f;
            if (camera->near_plane < 0.001f) camera->near_plane = 0.001f;
            if (camera->near_plane > camera->far_plane - 0.01f) camera->near_plane = camera->far_plane - 0.01f;
            std::cout << "Camera near: " << camera->near_plane << std::endl;
            break;
        case CAM_FAR:
            camera->far_plane -= 10.0f;
            if (camera->far_plane < camera->near_plane + 1.0f) camera->far_plane = camera->near_plane + 1.0f;
            std::cout << "Camera far: " << camera->far_plane << std::endl;
            break;
        case CAM_FOV:
            camera->fov -= 1.0f;
            if (camera->fov < 1.0f) camera->fov = 1.0f;
            std::cout << "Camera fov: " << camera->fov << std::endl;
            break;
        default: break;
        }
        camera->UpdateProjectionMatrix();
        break;
    }
    }
}


void Application::OnMouseMove(SDL_MouseButtonEvent event)
{
	int x = event.x; //posició actual del ratolí en X
	int y = event.y; //posició actual del ratolí en Y
	int xrel = x - last_mouse_x; //diferència de moviment del ratolí en X des de l'último evento
	int yrel = y - last_mouse_y; //diferència de moviment del ratolí en Y des de l'último evento
	last_mouse_x = x; //actualitzar la última posición del ratolí en X
	last_mouse_y = y; //actualitzar la última posición del ratolí en Y

    //només modes 3D
    if (current_mode == 1 || current_mode == 2 || current_mode == 3) return;

    int buttons = SDL_GetMouseState(NULL, NULL);

    //ORBITAR amb click esquerre
	if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) { //si el botó esquerre està premut --> orbitar la càmera al voltant del punt central de la càmera (camera->center) en funció del moviment del ratolí (xrel, yrel)
        float sensitivity = 0.005f;
        float angY = -xrel * sensitivity;
        float angX = -yrel * sensitivity;

		Vector3 dir = camera->eye - camera->center; //vector direcció des del punt central de la càmera fins a la posició de la càmera (camera->eye)

		Matrix44 rotY; //matriu de rotació al voltant de l'eix Y per orbitar horitzontalment
		rotY.MakeRotationMatrix(angY, Vector3(0, 1, 0)); //aplicar rotació al vector direcció per orbitar horitzontalment
		dir = rotY.RotateVector(dir); //rotar el vector direcció al voltant de l'eix Y

		Vector3 right = dir.Cross(camera->up).Normalize(); //vector right perpendicular al vector direcció i al vector up de la càmera, normalitzat per orbitar verticalment
		Matrix44 rotX; //matriu de rotació al voltant del vector right per orbitar verticalment
		rotX.MakeRotationMatrix(angX, right); //aplicar rotació al vector direcció per orbitar verticalment
		dir = rotX.RotateVector(dir); //rotar el vector direcció al voltant del vector right

		camera->eye = camera->center + dir; //actualitzar la posició de la càmera (camera->eye) sumant el vector direcció rotat al punt central de la càmera (camera->center) per orbitar al voltant del punt central
    }
    //PAN amb click dret
    else if (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)){
        //panSpeed escala amb la distància per moure's proporcionalment
        float dist = camera->eye.Distance(camera->center);
        float panSpeed = dist * 0.001f;

        Vector3 forward = (camera->center - camera->eye).Normalize(); 
        Vector3 right = forward.Cross(camera->up).Normalize();
        Vector3 up = right.Cross(forward).Normalize();

        Vector3 delta = (right * (float)(-xrel)) * panSpeed + (up * (float)(yrel)) * panSpeed;
		camera->eye += delta; //moure la posició de la càmera (camera->eye) sumant el delta calculat per panear horitzontalment i verticalment
		camera->center += delta; //moure el punt central de la càmera (camera->center) sumant el mateix delta per mantenir la direcció de la càmera mentre paneja
    }
}

void Application::OnWheel(SDL_MouseWheelEvent event) { //ZOOM AMB RODA DEL RATOLÍ
	float dy = event.preciseY; //moviment vertical de la roda del ratolí, amb precisió flotant, positiu cap a fora (alejando) i negatiu cap a dentro (acercando)

    //només als modes 3D (no als quads)
    if (current_mode == 1) return;

    float zoom_speed = 0.1f;
    float dist = camera->eye.Distance(camera->center);
    float new_distance = dist - (dy * dist * zoom_speed); // escala amb la distància

	if (new_distance < 2.0f) new_distance = 2.0f; //limitar la distància mínima per evitar que la càmera passi al punt central i es giri
	if (new_distance > 100.0f) new_distance = 100.0f; //limitar la distància máxima per evitar que la càmera s'allunyi massa i perdi el punt central de vista

    Vector3 direction = (camera->center - camera->eye).Normalize();
    camera->eye = camera->center - (direction * new_distance);
}


void Application::OnMouseButtonDown(SDL_MouseButtonEvent event) { //CLICK DEL RATOLI (PRESS)
	//actualitzar la posició del ratolí i el seu estat
    last_mouse_x = event.x;
    last_mouse_y = event.y;
}


void Application::OnMouseButtonUp(SDL_MouseButtonEvent event) { //CLICK DEL RATOLI (RELEASE)
}


void Application::OnFileChanged(const char* filename)//detecta canvi en un fitxer
{ 
	Shader::ReloadSingleShader(filename);
}