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
}

void Application::Init(void) { //inicialitza l'aplicació
	std::cout << "Initiating app..." << std::endl;

	//--------------------LAB2-----------------------
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
		Matrix44 m_trans;
		m_trans.MakeTranslationMatrix((i - 1) * 6.0f, 0.0f, 0.0f);

		Matrix44 m_rot;
		m_rot.MakeRotationMatrix(3.14159f, Vector3(1, 0, 0)); // Rotació 180° en X per posar-lo dret

		Matrix44 m_scale;
		float scale_factor = 5.0f;
		m_scale.MakeScaleMatrix(scale_factor, scale_factor, scale_factor);

		// Ordre: Model = Translation * Rotation * Scale (T * R * S)
		ent->model = m_trans * m_rot * m_scale;

		ent->time = i * 1.0f;
		entities.push_back(ent);
	}

	camera = new Camera();

	// POSICIÓ INICIAL 
	camera->LookAt(
		Vector3(0, -1, 8),    // Eye: 
		Vector3(0, -1, 0),     // Center: 
		Vector3(0, 1, 0)      // Up: 
	);

	//configuració de perspectiva 
	camera->SetPerspective(
		45.0f,                              //FOV
		window_width / (float)window_height, //aspect ratio
		0.1f,                               //Near plane 
		200.0f                              //Far plane 
	);
}

void Application::Render(void) { //renderitza l'aplicació
	
	//camera SETPERSPECTIVE: fov, aspect ratio, planol daprop, planol llunya
	camera->SetPerspective(camera->fov, window_width / (float)window_height, camera->near_plane, camera->far_plane);

	//camera LOOKAT: posició de la càmera, punt al que mira i vector up
	camera->LookAt(camera->eye, camera->center, camera->up);

	//MODE 2: dibuixar VARIES ENTITATS animades (mode d'animació)
	if (current_mode == 2) { 
        // Crear Z-buffer cada frame 
        FloatImage zbuffer(framebuffer.width, framebuffer.height); 
        zbuffer.Fill(999999.0f); // o FLT_MAX
		framebuffer.Fill(Color::BLACK); //esborrar pantalla a negre
        for (size_t i = 0; i < entities.size(); ++i) { //dibuixar les 3 entitats 
            //colors de les entitats: blau, verd, vermell
            Color col = Color::WHITE;
			if (i % 3 == 0) col = Color::BLUE; //entitat 0 = blau
			else if (i % 3 == 1) col = Color::GREEN; //entitat 1 = verd
			else if (i % 3 == 2) col = Color::RED; //entitat 2 = vermell
            entities[i]->Render(&framebuffer, camera, &zbuffer); //renderitzar l'entitat al framebuffer amb la càmera i color
        }
		framebuffer.Render();//mostrar framebuffer a la finestra
		return;//SORTIR
	}

    //MODE 1: dibuixar UNA SOLA ENTITAT (sense animació)
	else if (current_mode == 1) {
		framebuffer.Fill(Color::BLACK);
        // Crear Z-buffer 
        FloatImage zbuffer(framebuffer.width, framebuffer.height); 
        zbuffer.Fill(999999.0f);
		if (entities.size() >= 2) {
			entities[1]->Render(&framebuffer, camera, &zbuffer);  // La del mig → centre
		}
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

	case SDLK_1: //mode 1 = una sola entitat sense animació
        current_mode = 1;
        std::cout << "Single entity mode" << std::endl;
        break;

	case SDLK_2: //mode 2 = múltiples entitats animades
        current_mode = 2;
        std::cout << "Multiple animated entities mode" << std::endl;
        break;

	case SDLK_n: //N = near 
        current_camera_property = CAM_NEAR;
        std::cout << "Camera property: NEAR" << std::endl;
        break;

	case SDLK_f: //F = far
        current_camera_property = CAM_FAR;
        std::cout << "Camera property: FAR" << std::endl;
        break;

	case SDLK_v: //FOV = camp de visió
        current_camera_property = CAM_FOV;
        std::cout << "Camera property: FOV" << std::endl;
        break;

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
			std::cout << "Camera near: " << camera->near_plane << std::endl; //missatge del nou planol de prop
            break;

		case CAM_FAR: //augmentar el planol de lluny de la càmera
            camera->far_plane += 10.0f;
			std::cout << "Camera far: " << camera->far_plane << std::endl; //missatge del nou planol de lluny
            break;

		case CAM_FOV: //augmentar el camp de visió de la càmera
            camera->fov += 1.0f;
            if (camera->fov > 179.0f)
                camera->fov = 179.0f;
			std::cout << "Camera fov: " << camera->fov << std::endl; //missatge de la nova FOV
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
		if (current_camera_property == CAM_NONE) //si no s'ha seleccionat cap propietat de la càmera, no fer res
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


// Handle mouse motion events (used for camera orbit/pan)
void Application::OnMouseMove(SDL_MouseButtonEvent event) { //MOVIMENT DEL RATOLI
    int x = event.x;
    int y = event.y;

    int xrel = x - last_mouse_x;
    int yrel = y - last_mouse_y;

    last_mouse_x = x;
    last_mouse_y = y;

    // Only process when in 3D modes
    if (current_mode == 0) return;

    // get current mouse buttons state
    int buttons = SDL_GetMouseState(NULL, NULL);

    // Orbit with left button
    if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        float sensitivity = 0.005f;
        float angY = -xrel * sensitivity;
        // invert vertical rotation sign so moving mouse up rotates camera up
        float angX = yrel * sensitivity;

        // vector from center to eye
        Vector3 dir = camera->eye - camera->center;

        // rotate around up (Y) axis
        Matrix44 rotY; rotY.MakeRotationMatrix(angY, camera->up);
        dir = rotY.RotateVector(dir);

        // compute right axis and rotate around it
        Vector3 right = camera->up.Cross(dir).Normalize();
        Matrix44 rotX; rotX.MakeRotationMatrix(angX, right);
        dir = rotX.RotateVector(dir);

        camera->eye = camera->center + dir;
    }
    // Pan with right button
    else if (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        float panSpeed = 0.01f;
        Vector3 forward = (camera->center - camera->eye).Normalize();
        Vector3 right = forward.Cross(camera->up).Normalize();

        // Invert vertical mouse delta to make upward mouse movement pan the camera up
        camera->eye += (-xrel * panSpeed) * right + (-yrel * panSpeed) * camera->up;
        camera->center += (-xrel * panSpeed) * right + (-yrel * panSpeed) * camera->up;
    }
}

void Application::OnMouseButtonDown(SDL_MouseButtonEvent event) { //CLICK DEL RATOLI (PRESS)
    // Reset last mouse when starting any mouse interaction to avoid jump deltas
    last_mouse_x = event.x;
    last_mouse_y = event.y;
}


void Application::OnMouseButtonUp(SDL_MouseButtonEvent event) { //CLICK DEL RATOLI (RELEASE)
}



void Application::OnWheel(SDL_MouseWheelEvent event) //ZOOM AMB LA RODA DEL RATOLI
{
	float dy = event.preciseY;

	// Zoom amb la roda del ratolí (modes 1 i 2)
	if (current_mode != 0)
	{
		float zoom_speed = 1.0f; // Velocitat de zoom

		// Vector de direcció normalitzat (des de l'eye cap al center)
		Vector3 direction = (camera->center - camera->eye).Normalize();

		// Calcular la distància actual
		float current_distance = camera->eye.Distance(camera->center);

		// Evitar que la càmera passi pel centre o s'allunyi massa
		float new_distance = current_distance - (dy * zoom_speed);

		// Limitar la distància mínima i màxima
		if (new_distance < 2.0f) new_distance = 2.0f;     // No més aprop de 2 unitats
		if (new_distance > 50.0f) new_distance = 50.0f;   // No més lluny de 50 unitats

		// Actualitzar la posició de l'eye mantenint la direcció
		camera->eye = camera->center - (direction * new_distance);
	}
}

void Application::OnFileChanged(const char* filename)//detecta canvi en un fitxer
{ 
	Shader::ReloadSingleShader(filename);
}