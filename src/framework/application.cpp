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
	Mesh* m = new Mesh(); //crear nova malla
	m->LoadOBJ("meshes/lee.obj"); //carregar malla de lee (2.4)

    //crear les 3 entitats amb la malla carregada i posar-les a diferents posicions
	for (int i = 0; i < 3; i++) {// i < x --> x = número d'entitats a crear
        Entity* ent = new Entity(m); //crear entitat amb la malla carregada

		//inicialitzar la matriu de model per a cada entitat: translació + rotació + escala
		Matrix44 m_trans; m_trans.MakeTranslationMatrix((i - 1) * 6.0f, 0.0f, 0.0f); //matriu de translació --> per separar les entitats (posar a -6, 0, 0), (0, 0, 0) i (6, 0, 0)
		Matrix44 m_rot; m_rot.MakeRotationMatrix(3.14159f, Vector3(1, 0, 0)); //matriu de rotació --> per posar la malla dreta (rotar 180 graus al voltant de l'eix X)
		Matrix44 m_scale; //matriu d'escala --> per veure millor les malles 

		float scale_factor = 3.0f; //factor d'escala per a les entitats --> (sx, sy, sz) = (3, 3, 3)
		m_scale.MakeScaleMatrix(scale_factor, scale_factor, scale_factor); //matriu d'escala amb el factor d'escala (sx, sy, sz)

        // Ordre: Model = Translation * Rotation * Scale (T * R * S)
		ent->model = m_trans * m_rot * m_scale; //matriu de modelat = translació * rotació * escala

        ent->time = i * 1.0f; //desfasament temporal per variar animacions
        entities.push_back(ent); //afegir l'entitat al vector d'entitats per a que es renderitzi després 
    }
	camera = new Camera(); //nova càmera
    // Eye (on estàs), Center (on mires), Up (quin eix és el cel)

	//camera LOOKAT: posició de la càmera, punt al que mira i vector up
	camera->LookAt(Vector3(0, 5, 12), Vector3(0, 0, 0), Vector3(0, 1, 0)); //posició inicial de la càmera

	//camera PERSPECTIVE: fov, aspect ratio, planol daprop, planol llunya
	camera->SetPerspective(60, window_width / (float)window_height, 0.1f, 900.0f); //projecció inicial de la càmera
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
		framebuffer.Fill(Color::BLACK); //esborrar pantalla a negre
		if (!entities.empty()) { //comprovar que hi ha entitats carregades
			// dibuixar només la primera entitat
			entities[0]->Render(&framebuffer, camera, Color::WHITE); //renderitzar l'entitat al framebuffer amb la càmera i color blanc
		}
		framebuffer.Render(); //mostrar framebuffer a la finestra
		return; //SORTIR
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



void Application::OnMouseMove(SDL_MouseButtonEvent event) { //MOVIMENT DEL RATOLI
	static int last_x = event.x;
	static int last_y = event.y;

	int xrel = event.x - last_x;
	int yrel = event.y - last_y;

	last_x = event.x;
	last_y = event.y;

	// LAB1: Si estem en mode Paint (0) fem la funcionalitat de dibuix
	if (current_mode == 0) {
		Vector2 mouse(event.x, window_height - event.y);
		current_pos = mouse;

		if (is_drawing) {
			Color draw_color = (current_tool == TOOL_ERASER) ? Color::WHITE : current_color;

			switch (current_tool) {
			case TOOL_PENCIL:
			case TOOL_ERASER:
				framebuffer.DrawLineDDA(start_pos.x, start_pos.y, current_pos.x, current_pos.y, draw_color);
				start_pos = current_pos;
				break;
			default:
				break;
			}
		}
	}

	//LAB2: MODE 1/2: CÀMERA (single o multiple entities)
	else
	{
		int mx, my;
		Uint32 buttons = SDL_GetMouseState(&mx, &my);

		// BOTÓ ESQUERRE (ORBIT - Rotació de l'EYE al voltant del CENTER)
		if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT))
		{
			float sensitivity = 0.01f;

			//obtenim el vector de visió (des del centre cap a l'ull)
			Vector3 view = camera->eye - camera->center;

			//rotació Horitzontal (Eix Y global)
			Matrix44 rot_y;
			rot_y.MakeRotationMatrix(-xrel * sensitivity, Vector3(0, 1, 0));
			view = rot_y.RotateVector(view);

			//rotació Vertical (Eix local "right")
			Vector3 right = view.Cross(camera->up).Normalize();
			Matrix44 rot_x;
			rot_x.MakeRotationMatrix(-yrel * sensitivity, right);
			view = rot_x.RotateVector(view);

			//actualitzar la posició de l'ull mantenint el centre fix
			camera->eye = camera->center + view;
		}

		// BOTÓ DRET (PANNING - Mou el CENTER i l'EYE junts)
		else if (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT))
		{
			//calculem els vectors de la càmera
			Vector3 front = (camera->center - camera->eye).Normalize();
			Vector3 right = front.Cross(camera->up).Normalize();
			Vector3 top = right.Cross(front).Normalize();

			//velocitat proporcional a la distància
			float speed = 0.01f * camera->eye.Distance(camera->center);

			//desplaçament lateral i vertical
			Vector3 movement = (right * -xrel * speed) + (top * yrel * speed);

			//aplicar el moviment a ambdós (eye i center) per mantenir l'orientació
			camera->eye = camera->eye + movement;
			camera->center = camera->center + movement;
		}
	}
}


void Application::OnWheel(SDL_MouseWheelEvent event) //detecta scroll del ratoli
{
	float dy = event.preciseY;

	//zoom amb la roda del ratolí: moure la càmera cap a dins/fora al llarg de la línia de visió (modes 1 i 2)
	if (current_mode != 0) //nomes modes 1 i 2 (no mode paint)
	{
		float zoom_speed = 0.02f; //velocitat 

		//vector de direcció de la càmera (des del centre fins a l'ull)
		Vector3 view_vector = camera->center - camera->eye;

		//moure el eye de la càmera al llarg del vector de visió segons la direcció del scroll
			//si dy és positiu (rodeta amunt), sumem vector -> ens apropem
			//si dy és negatiu (rodeta avall), restem vector -> ens allunyem
		camera->eye = camera->eye + (view_vector * dy * zoom_speed);
	}
}

void Application::OnFileChanged(const char* filename)//detecta canvi en un fitxer
{ 
	Shader::ReloadSingleShader(filename);
}