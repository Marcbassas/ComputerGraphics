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

Application::~Application()
{
}

void Application::Init(void)
{
	std::cout << "Initiating app..." << std::endl;
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
}
*/

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

void Application::OnMouseButtonDown(SDL_MouseButtonEvent event)//detecta click del ratoli (press)
{
	if (event.button == SDL_BUTTON_LEFT) {

	}
}

void Application::OnMouseButtonUp(SDL_MouseButtonEvent event)//detecta click del ratoli (release)
{
	if (event.button == SDL_BUTTON_LEFT) {

	}
}

void Application::OnMouseMove(SDL_MouseButtonEvent event)//detecta moviment del ratoli
{
	
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