#include "application.h"
#include "mesh.h"
#include "shader.h"
#include "utils.h" 

Application::Application(const char* caption, int width, int height)
{
	this->window = createWindow(caption, width, height); 

	int w,h;
	SDL_GetWindowSize(window,&w,&h);

	this->mouse_state = 0;
	this->time = 0.f;
	this->window_width = w;
	this->window_height = h;
	this->keystate = SDL_GetKeyboardState(nullptr);

	this->framebuffer.Resize(w, h);
}

Application::~Application(){ 
}

void Application::Init(void) { //inicialitza l'aplicació
	std::cout << "Initiating app..." << std::endl;

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

	framebuffer.Fill(Color::WHITE); //inicialitzar el framebuffer a blanc
	preview_framebuffer = framebuffer; //inicialitzar el framebuffer de previsualització

	rain.Init(window_width, window_height); //2.3 --> inicialitzar la pluja

	//LAB2
	// 1. Carreguem la malla (una sola vegada per estalviar memòria!)
	Mesh* m = new Mesh();
	m->LoadOBJ("meshes/lee.obj");

	// 2. Creem 3 entitats i les fiquem al vector
	for (int i = 0; i < 3; i++) {
		Entity* ent = new Entity(m);
		// MakeTranslationMatrix reinicia la matriu i posa la posició
		ent->model.MakeTranslationMatrix((i - 1) * 15.0f, 0.0f, 0.0f);
		entities.push_back(ent);
	}
	camera = new Camera();
	// Eye (on estàs), Center (on mires), Up (quin eix és el cel)
	camera->LookAt(Vector3(0, 10, 20), Vector3(0, 0, 0), Vector3(0, 1, 0));
	camera->SetPerspective(45, window_width / (float)window_height, 0.1, 1000);
}


/*
// Render one frame
//pinta el framebuffer a la finestra
void Application::Render(void) {
	//prova del DrawLineDDA
	framebuffer.Fill(Color::BLACK);
	//centre de la finestra
	int x = window_width / 2;
	int y = window_height / 2;
	Color color = Color::WHITE; //escollir color blanc
	framebuffer.DrawLineDDA(x, y, x + 100 * cos(time), y + 100 * sin(time), color);
	framebuffer.Render();
}
*/

/*
//Render Triangle 2.1.3
void Application::Render(void) {
	/*
	// 1. Netejar la pantalla
	framebuffer.Fill(Color::BLACK);

	// 2. Definim 3 punts quiets:	
	Vector2 p0(400, 100); // Punt de dalt (centre)
	Vector2 p1(200, 450); // Punt de baix esquerra
	Vector2 p2(600, 450); // Punt de baix dreta

	// 3. Dibuixem el triangle estàtic
	framebuffer.DrawTriangle(p0, p1, p2, Color::WHITE, true, Color::RED);

	// 4. Mostrar a la finestra
	framebuffer.Render();
	
	framebuffer.Fill(Color::WHITE); // Dibuixar botons 
	for (auto& b : buttons) 
		b.Render(framebuffer); 
	framebuffer.Render();
}
*/
void Application::Render(void) { //renderitza l'aplicació
//---ANIMACIÓ DE PARTÍCULES---
	if (current_mode == 2) { //si estem en mode 2 (animació)
		framebuffer.Fill(Color::BLACK); //esborrar pantalla a negre
		rain.Render(&framebuffer);   //dibuixar gotes
		// LAB2: Dibuixar les 3 entitats ---
		for (Entity* ent : entities) {
			// Passem la càmera i un color (per exemple, Blanc)
			ent->Render(&framebuffer, camera, Color::WHITE);
		}
		framebuffer.Render();   //mostrar framebuffer a la finestra
		return;     //SORTIM (no dibuixem res més)
	}

	//codi del paint
	preview_framebuffer = framebuffer; //copiar el framebuffer al de previsualització

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
}





// Called after render
//actualittza l'aplicacio en funcio del temps que ha passat
void Application::Update(float seconds_elapsed){
	// Actualitzem el temps global de l'aplicació (molt útil per a sinus/cosinus)
	this->time += seconds_elapsed;

	//si el mode actual és 2 (animació) --> actualitzar la pluja
	if (current_mode == 2) {
		rain.Update(seconds_elapsed, window_width, window_height); //update(temps, amplada finestra, altura finestra)
		// 2. Actualitzar les entitats 3D (Nou per al punt 2.4)
		for (int i = 0; i < entities.size(); ++i) {
			entities[i]->Update(seconds_elapsed);
		}
	}
}

//keyboard press event 
void Application::OnKeyPressed(SDL_KeyboardEvent event)//detecta tecla premuda 
{
	// KEY CODES: https://wiki.libsdl.org/SDL2/SDL_Keycode
	switch(event.keysym.sym) {

		case SDLK_ESCAPE: exit(0); break; // ESC key, kill the app
		//1 --> paint mode
		case SDLK_1: 
			current_mode = 1; 
			std::cout << "Paint mode" << std::endl; 
			break;

		//2 --> animation mode
		case SDLK_2: 
			current_mode = 2; 
			std::cout << "Animation mode" << std::endl; 
			break;

		//F --> toggle fill shapes
		case SDLK_f:
			fill_shapes = !fill_shapes; 
			std::cout << "Fill shapes: " << (fill_shapes ? "ON" : "OFF") << std::endl; 
			break;

		//+ --> increase border width
		case SDLK_PLUS:
		case SDLK_EQUALS: //perquè el + està al mateix botó que =
			border_width++;
			std::cout << "Border width: " << border_width << std::endl;
			break;

		//- --> decrease border width
		case SDLK_MINUS:
			border_width--;
			std::cout << "Border width: " << border_width << std::endl;
			break;
	}
}

void Application::OnMouseButtonDown(SDL_MouseButtonEvent event) { //CLICK DEL RATOLI (PRESS)
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


void Application::OnMouseButtonUp(SDL_MouseButtonEvent event) { //CLICK DEL RATOLI (RELEASE)
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



void Application::OnMouseMove(SDL_MouseButtonEvent event) { //MOVIMENT DEL RATOLI
	Vector2 mouse(event.x, window_height - event.y); //convertir coordenades del ratolí
	current_pos = mouse; //actualitzar la posició actual del ratolí

	if (is_drawing) { //si s'està dibuixant
		Color draw_color = (current_tool == TOOL_ERASER) ? Color::WHITE : current_color; //color de dibuix (blanc si és goma, sinó el color actual)

		switch (current_tool) {
		case TOOL_PENCIL: //llapis
		case TOOL_ERASER: //goma
			framebuffer.DrawLineDDA(start_pos.x, start_pos.y, current_pos.x, current_pos.y, draw_color); //dibuixar línia des de la posició inicial fins a la actual
			start_pos = current_pos; //actualitzar la posició inicial a la actual
			break;
		default: 
			break; //no fer res
		}
	}
}


void Application::OnWheel(SDL_MouseWheelEvent event)//detecta scroll del ratoli --> zoom in/out
{
	float dy = event.preciseY;

	// ...
}

void Application::OnFileChanged(const char* filename)//detecta canvi en un fitxer
{ 
	Shader::ReloadSingleShader(filename);
}