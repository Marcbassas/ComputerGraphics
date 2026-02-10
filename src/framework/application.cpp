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

    //--------------------LAB1-----------------------
    /*
    //carregar icones
    Image pencil_img;  pencil_img.LoadPNG("images/pencil.png");
    Image line_img;    line_img.LoadPNG("images/line.png");
    Image rect_img;    rect_img.LoadPNG("images/rectangle.png");
    Image tri_img;     tri_img.LoadPNG("images/triangle.png");
    Image eraser_img;  eraser_img.LoadPNG("images/eraser.png");
    Image clear_img;   clear_img.LoadPNG("images/clear.png");
    Image load_img;    load_img.LoadPNG("images/load.png");
    Image save_img;    save_img.LoadPNG("images/save.png");
    Image red_img;     red_img.LoadPNG("images/red.png");
    Image green_img;   green_img.LoadPNG("images/green.png");
    Image blue_img;    blue_img.LoadPNG("images/blue.png");

    int x = 10; //posició fixa
    int y = 10; //posició inicial
    int sep = 5; //separació entre botons

    //EINES --> Button(icona, posició, tipus)
    buttons.push_back(Button(pencil_img, Vector2(x, y), BUTTON_PENCIL)); y += pencil_img.height + sep; //llapis
    buttons.push_back(Button(line_img, Vector2(x, y), BUTTON_LINE));   y += line_img.height + sep; //línia
    buttons.push_back(Button(rect_img, Vector2(x, y), BUTTON_RECT));   y += rect_img.height + sep; //rectangle
    buttons.push_back(Button(tri_img, Vector2(x, y), BUTTON_TRIANGLE)); y += tri_img.height + sep; //triangle
    buttons.push_back(Button(eraser_img, Vector2(x, y), BUTTON_ERASER)); y += eraser_img.height + sep; //goma

    y += 10; //separació entre grups

    //COLORS
    buttons.push_back(Button(red_img, Vector2(x, y), BUTTON_COLOR_RED));   y += red_img.height + sep; //boto color vermell
    buttons.push_back(Button(green_img, Vector2(x, y), BUTTON_COLOR_GREEN)); y += green_img.height + sep; //boto color verd
    buttons.push_back(Button(blue_img, Vector2(x, y), BUTTON_COLOR_BLUE));  y += blue_img.height + sep; //boto color blau

    y += 10; // separació entre grups

    // ACCIONS
    buttons.push_back(Button(clear_img, Vector2(x, y), BUTTON_CLEAR)); y += clear_img.height + sep; //boto netejar
    buttons.push_back(Button(load_img, Vector2(x, y), BUTTON_LOAD));  y += load_img.height + sep; //boto carregar
    buttons.push_back(Button(save_img, Vector2(x, y), BUTTON_SAVE)); //boto desar

    framebuffer.Fill(Color::BLACK); //inicialitzar el framebuffer a negre per al LAB 2
    preview_framebuffer = framebuffer; //inicialitzar el framebuffer de previsualització
    */

	//--------------------LAB2-----------------------
	//INICIALITZACIO DE LA MALLA, ENTITATS I CÀMERA
	Mesh* m = new Mesh();
	m->LoadOBJ("meshes/lee.obj");

	//crear les 3 entitats amb la malla carregada i posar-les a diferents posicions
	for (int i = 0; i < 3; i++) {
		Entity* ent = new Entity(m);

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

	// POSICIÓ INICIAL MILLOR: més aprop i mirant directament al centre
	camera->LookAt(
		Vector3(0, 8, 25),    // Eye: més aprop (era 12, ara 15 està bé, o prova amb 10-12 per més aprop)
		Vector3(0, 3, 0),     // Center: mirant al centre on estan les entitats
		Vector3(0, 1, 0)      // Up: eix Y cap amunt
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
		framebuffer.Fill(Color::BLACK); //esborrar pantalla a negre
        for (size_t i = 0; i < entities.size(); ++i) { //dibuixar les 3 entitats 
            //colors de les entitats: blau, verd, vermell
            Color col = Color::WHITE;
			if (i % 3 == 0) col = Color::BLUE; //entitat 0 = blau
			else if (i % 3 == 1) col = Color::GREEN; //entitat 1 = verd
			else if (i % 3 == 2) col = Color::RED; //entitat 2 = vermell
            entities[i]->Render(&framebuffer, camera, col); //renderitzar l'entitat al framebuffer amb la càmera i color
        }
		framebuffer.Render();//mostrar framebuffer a la finestra
		return;//SORTIR
	}

    //MODE 1: dibuixar UNA SOLA ENTITAT (sense animació)
	else if (current_mode == 1) {
		framebuffer.Fill(Color::BLACK);
		if (entities.size() >= 2) {
			entities[1]->Render(&framebuffer, camera, Color::WHITE);  // La del mig → centre
		}
		framebuffer.Render();
		return;
	}

	//-----------------LAB1: EINA DE DIBUIX---------------------------
	/*preview_framebuffer = framebuffer; //copiar el framebuffer al de previsualització

	if (is_drawing && (start_pos.x != current_pos.x || start_pos.y != current_pos.y)) { //si s'està dibuixant i la posició inicial és diferent de l'actual
		Color preview = Color(0.0f, 0.0f, 0.0f); //color de previsualització

		switch (current_tool) { //eina actual
		case TOOL_LINE: //linia
			preview_framebuffer.DrawLineDDA(start_pos.x, start_pos.y, current_pos.x, current_pos.y, preview); //dibuixar línia de previsualització
			break;

		case TOOL_RECT: { //rectangle
			int x = std::min(start_pos.x, current_pos.x); //x minima
			int y = std::min(start_pos.y, current_pos.y); //y minima
			int w = std::abs(current_pos.x - start_pos.x); //amplada
			int h = std::abs(current_pos.y - start_pos.y); //altura
			preview_framebuffer.DrawRect(x, y, w, h, preview, border_width, fill_shapes, preview); //dibuixar rectangle de previsualització
			break;
		}

		case TOOL_TRIANGLE: { //triangle 
			Vector2 p0 = start_pos; //punt superior
			Vector2 p1(current_pos.x, start_pos.y); //punt inferior dret
			Vector2 p2((start_pos.x + current_pos.x) * 0.5f, current_pos.y); //punt inferior esquerra
			preview_framebuffer.DrawTriangle(p0, p1, p2, preview, fill_shapes, preview); //dibuixar triangle de previsualització 
			break;
		}

		default:
			break; //no fer res
		}
        else if (event.button == SDL_BUTTON_RIGHT) {
            // reset last mouse sample to avoid large jump when starting panning
            last_mouse_x = event.x;
            last_mouse_y = event.y;
        }
	}

	for (auto& b : buttons) //renderitzar tots els botons
		b.Render(preview_framebuffer); //dibuixar botó al framebuffer de previsualització

	preview_framebuffer.Render(); //mostrar el framebuffer de previsualització a la finestra
    */
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


void Application::OnMouseButtonDown(SDL_MouseButtonEvent event) { //CLICK DEL RATOLI (PRESS)
    // Reset last mouse when starting a mouse interaction to avoid jump deltas
    if (event.button == SDL_BUTTON_RIGHT) {
        last_mouse_x = event.x;
        last_mouse_y = event.y;
    }

    //LAB1 - Només processar l'inici de dibuix si estem en mode Paint (mode 0)
    /*if (current_mode == 0) {
        if (event.button == SDL_BUTTON_LEFT) { //si és el botó esquerre
            Vector2 mouse(event.x, window_height - event.y); 
            for (auto& b : buttons) { //recorre=er tots els botons
                if (b.IsMouseInside(mouse)) {
                    //reset de preview per evitar figures fantasma
                    is_drawing = false; //indicar que no s'està dibuixant
                    start_pos = Vector2(-1, -1); //posició inicial fora de la pantalla
                    current_pos = Vector2(-1, -1); //posició actual fora de la pantalla
                    switch (b.type) { //tipus de botó
                    case BUTTON_PENCIL: current_tool = TOOL_PENCIL; break; 
                    case BUTTON_LINE: current_tool = TOOL_LINE; break;
                    case BUTTON_RECT: current_tool = TOOL_RECT; break;
                    case BUTTON_TRIANGLE: current_tool = TOOL_TRIANGLE; break;
                    case BUTTON_ERASER: current_tool = TOOL_ERASER; break;

                    case BUTTON_COLOR_RED: current_color = Color::RED; break;
                    case BUTTON_COLOR_GREEN: current_color = Color::GREEN; break;
                    case BUTTON_COLOR_BLUE: current_color = Color::BLUE; break;

                    case BUTTON_CLEAR: framebuffer.Fill(Color::WHITE); break;
                    case BUTTON_LOAD: framebuffer.LoadPNG("images/fruits.png"); break;
                    case BUTTON_SAVE: framebuffer.SaveTGA("output.tga"); break;
                    }
                    return;
                }


            }

            //si no ha clicat cap botó → comencem a dibuixar
            is_drawing = true; //indicar que s'està dibuixant
            start_pos = mouse; //guardar la posició inicial del ratolí
            current_pos = mouse; //guardar la posició actual del ratolí
        }
    }
    */
}


void Application::OnMouseButtonUp(SDL_MouseButtonEvent event) { //CLICK DEL RATOLI (RELEASE)
	//LAB1 - Només processar el final del dibuix si estem en mode Paint (mode 0)
    /*if (current_mode == 0) {
        if (event.button == SDL_BUTTON_LEFT) { //si és el botó esquerre
            is_drawing = false; //indicar que s'ha deixat de dibuixar

            Color draw_color = (current_tool == TOOL_ERASER) ? Color::WHITE : current_color; //color de dibuix (blanc si és goma, sinó el color actual)

            switch (current_tool) { //eina actual
            case TOOL_LINE: //línia
                framebuffer.DrawLineDDA(start_pos.x, start_pos.y, current_pos.x, current_pos.y, draw_color);
                break;

            case TOOL_RECT: { //rectangle --> variables per a les dimensions(4)
                int x = std::min(start_pos.x, current_pos.x);
                int y = std::min(start_pos.y, current_pos.y);
                int w = std::abs(current_pos.x - start_pos.x);
                int h = std::abs(current_pos.y - start_pos.y);
                framebuffer.DrawRect(x, y, w, h, draw_color, border_width, fill_shapes, draw_color);
                break;
            }

            case TOOL_TRIANGLE: { //triangle --> punts del triangle(3)
                Vector2 p0 = start_pos;
                Vector2 p1(current_pos.x, start_pos.y);
                Vector2 p2((start_pos.x + current_pos.x) * 0.5f, current_pos.y);
                framebuffer.DrawTriangle(p0, p1, p2, draw_color, fill_shapes, draw_color);
                break;
            }
            }
        }
    }
	*/
}



void Application::OnMouseMove(SDL_MouseButtonEvent event)
{
    int xrel = event.x - last_mouse_x;
    int yrel = event.y - last_mouse_y;

    last_mouse_x = event.x;
    last_mouse_y = event.y;
    // ────────────────────────────────────────────────
    // Modes 1 i 2 → control de càmera 3D (LAB 2)
    // ────────────────────────────────────────────────
    int mx, my;
    Uint32 buttons = SDL_GetMouseState(&mx, &my);

    bool left_pressed = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    bool right_pressed = (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;

    float sensitivity = 0.005f;   // orbit sensitivity (ajusta si cal)

    if (left_pressed)   // ── ORBIT ── Botó esquerre premut
    {
        Vector3 view = camera->eye - camera->center;

        // Rotació horitzontal (Y global)
        Matrix44 rot_y;
        rot_y.MakeRotationMatrix(-xrel * sensitivity, Vector3(0, 1, 0));
        view = rot_y * view;

        // Rotació vertical (eix right local)
        Vector3 right = view.Cross(camera->up).Normalize();
        Matrix44 rot_x;
        rot_x.MakeRotationMatrix(-yrel * sensitivity, right);
        view = rot_x * view;

        camera->eye = camera->center + view;

        camera->UpdateViewMatrix();
    }

    if (right_pressed)   // ── PAN ── Botó dret premut
    {
        // Compute camera basis for panning
        Vector3 forward = (camera->center - camera->eye).Normalize();
        Vector3 right = forward.Cross(camera->up).Normalize();
        Vector3 up = right.Cross(forward).Normalize();

        float dist = (camera->eye - camera->center).Length();
        float pan_speed = 0.04f * dist;

        // delta in world units (mouse right -> pan right). invert Y to match screen coordinates
        Vector3 delta = (right * (xrel * pan_speed)) - (up * (yrel * pan_speed));

        // Move only the center (target) so the right-click pan updates the focus only
        camera->center += delta;

        camera->UpdateViewMatrix();
    }

    // Nota: si vols que el pan mogui eye i centre junts (com en alguns programes),
    // pots fer camera->eye += delta; també.
    // Però segons el lab: "The center (target) of the camera should be interactive using the mouse (right button)."
    // → només moure center és el que demana
}

void Application::OnWheel(SDL_MouseWheelEvent event)
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