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

	//Carregar icones
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

	int x = 10; // posició fixa
	int y = 10; // posició inicial
	int sep = 5; // separació entre botons

	//EINES
	buttons.push_back(Button(pencil_img, Vector2(x, y), BUTTON_PENCIL)); y += pencil_img.height + sep; //boto llapis
	buttons.push_back(Button(line_img, Vector2(x, y), BUTTON_LINE));   y += line_img.height + sep; //boto línia
	buttons.push_back(Button(rect_img, Vector2(x, y), BUTTON_RECT));   y += rect_img.height + sep; //boto rectangle
	buttons.push_back(Button(tri_img, Vector2(x, y), BUTTON_TRIANGLE)); y += tri_img.height + sep; //boto triangle
	buttons.push_back(Button(eraser_img, Vector2(x, y), BUTTON_ERASER)); y += eraser_img.height + sep; //boto goma

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

	framebuffer.Fill(Color::WHITE);

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
void Application::Render(void){
	for (auto& b : buttons) //dibuixar botons
		b.Render(framebuffer);
	if (is_drawing) { //si s'està dibuixant
		Color draw_color = (current_tool == TOOL_ERASER) ? Color::WHITE : current_color; //color de dibuix (blanc si és goma)
		switch (current_tool)
		{
		case TOOL_PENCIL: //llapis
			framebuffer.DrawLineDDA(start_pos.x, start_pos.y, current_pos.x, current_pos.y, draw_color);
			start_pos = current_pos;
			break;

		case TOOL_ERASER: //goma
			framebuffer.DrawLineDDA(start_pos.x, start_pos.y, current_pos.x, current_pos.y, Color::WHITE);
			start_pos = current_pos;
			break;

		default: 
			// Per LINE, RECT, TRIANGLE no fem res aquí
			break;
		}
	}

	framebuffer.Render();
}



// Called after render
//actualittza l'aplicacio en funcio del temps que ha passat
void Application::Update(float seconds_elapsed)
{

}

//keyboard press event 
void Application::OnKeyPressed(SDL_KeyboardEvent event)//detecta tecla premuda 
{
	// KEY CODES: https://wiki.libsdl.org/SDL2/SDL_Keycode
	switch(event.keysym.sym) {
		case SDLK_ESCAPE: exit(0); break; // ESC key, kill the app
	}
}

void Application::OnMouseButtonDown(SDL_MouseButtonEvent event) { //CLICK DEL RATOLI (PRESS)
	if (event.button == SDL_BUTTON_LEFT) { //si és el botó esquerre
		Vector2 mouse(event.x, window_height - event.y); 
		for (auto& b : buttons) { //recorre=er tots els botons
			if (b.IsMouseInside(mouse)){
				switch (b.type) { //comprovar tipus de botó
				case BUTTON_PENCIL: current_tool = TOOL_PENCIL; break; //canviar eina actual
				case BUTTON_LINE: current_tool = TOOL_LINE; break;
				case BUTTON_RECT: current_tool = TOOL_RECT; break;
				case BUTTON_TRIANGLE: current_tool = TOOL_TRIANGLE; break; 
				case BUTTON_ERASER: current_tool = TOOL_ERASER; break; 

				case BUTTON_COLOR_RED: current_color = Color::RED; break; //canviar color actual
				case BUTTON_COLOR_GREEN: current_color = Color::GREEN; break; 
				case BUTTON_COLOR_BLUE: current_color = Color::BLUE; break; 

				case BUTTON_CLEAR: framebuffer.Fill(Color::WHITE); break; //netejar imatge
				case BUTTON_LOAD: framebuffer.LoadPNG("images/canvas.png"); break; //carregar imatge des de "canvas.png"
				case BUTTON_SAVE: framebuffer.SaveTGA("output.tga"); break; //desar imatge a "output.tga"
				}

				return;
			}
		}

		// Si no ha clicat cap botó → comencem a dibuixar
		is_drawing = true; //indicar que s'està dibuixant
		start_pos = mouse; //guardar la posició inicial del ratolí
		current_pos = mouse; //guardar la posició actual del ratolí
	}
}


void Application::OnMouseButtonUp(SDL_MouseButtonEvent event) { //CLICK DEL RATOLI (RELEASE)
	if (event.button == SDL_BUTTON_LEFT) { //si és el botó esquerre
		is_drawing = false; //indicar que s'ha deixat de dibuixar
		Color draw_color = (current_tool == TOOL_ERASER) ? Color::WHITE : current_color; //color de dibuix (blanc si és goma)

		switch (current_tool) { //segons l'eina actual
		case TOOL_LINE: //dibuixar línia
			framebuffer.DrawLineDDA(start_pos.x, start_pos.y, current_pos.x, current_pos.y, draw_color);
			break;

		case TOOL_RECT: { //dibuixar rectangle
			int x = std::min(start_pos.x, current_pos.x);
			int y = std::min(start_pos.y, current_pos.y);
			int w = std::abs(current_pos.x - start_pos.x);
			int h = std::abs(current_pos.y - start_pos.y);
			framebuffer.DrawRect(x, y, w, h, draw_color, border_width, fill_shapes, draw_color);
			break;
		}

		case TOOL_TRIANGLE: { //dibuixar triangle
			Vector2 p0 = start_pos;
			Vector2 p1(current_pos.x, start_pos.y);
			Vector2 p2((start_pos.x + current_pos.x) * 0.5f, current_pos.y);
			framebuffer.DrawTriangle(p0, p1, p2, draw_color, fill_shapes, draw_color);
			break;
		}
		}
	}
}



void Application::OnMouseMove(SDL_MouseButtonEvent event){ //detecta el moviment del ratoli
	current_pos.set(event.x, window_height - event.y); // Convertim coordenades de SDL a les nostres (origen a baix a l'esquerra)
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