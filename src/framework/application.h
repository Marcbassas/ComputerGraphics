/*  
	+ This class encapsulates the application, is in charge of creating the data, getting the user input, process the update and render.
*/
#pragma once

#include "main/includes.h"
#include "framework.h"
#include "image.h"
//---------------- BUTTON CLASS ----------------
enum ButtonType { //tipus de botó 
	BUTTON_PENCIL, //botó llapis
	BUTTON_COLOR_RED, //botó color vermell
	BUTTON_COLOR_GREEN, //botó color verd
	BUTTON_COLOR_BLUE, //botó color blau
	BUTTON_ERASER, //botó goma
	BUTTON_TRIANGLE, //botó triangle
	BUTTON_LINE, //botó línia
	BUTTON_RECT, //botó rectangle 
	BUTTON_CLEAR, //botó netejar
	BUTTON_LOAD, //botó carregar
	BUTTON_SAVE //botó desar
};

class Button { //classe botó
public: //atributs
	Vector2 position; //posició del botó
	Image icon; //icona del botó
	ButtonType type; //tipus de botó	

	Button() {} //constructor
	Button(const Image& img, const Vector2& pos, ButtonType t) : icon(img), position(pos), type(t) { //constructor amb paràmetres
	}

	bool IsMouseInside(const Vector2& mouse) const { //comprova si el ratolí està dins del botó
		return mouse.x >= position.x && mouse.x < position.x + icon.width && mouse.y >= position.y && mouse.y < position.y + icon.height; //retorna true si està dins, false si no
	}

	void Render(Image& framebuffer) { //renderitza el botó
		framebuffer.DrawImage(icon, (int)position.x, (int)position.y); //dibuixa la icona del botó a la posició
	}
};

//---------------- TOOL STATE ----------------
enum ToolType {
	TOOL_PENCIL,
	TOOL_LINE,
	TOOL_RECT,
	TOOL_TRIANGLE,
	TOOL_ERASER
};

//---------------- APPLICATION CLASS ----------------
class Application
{
public:
	//variables globals per a l'eina actual, color, amplada del contorn i si omplir formes
	std::vector<Button> buttons;

	Image preview_framebuffer; //framebuffer de previsualització

	ToolType current_tool = TOOL_PENCIL;
	Color current_color = Color::WHITE;
	int border_width = 1;
	bool fill_shapes = false;

	bool is_drawing = false;
	Vector2 start_pos;
	Vector2 current_pos;

	// Window

	SDL_Window* window = nullptr;
	int window_width;
	int window_height;

	float time;

	// Input
	const Uint8* keystate;
	int mouse_state; // Tells which buttons are pressed
	Vector2 mouse_position; // Last mouse position
	Vector2 mouse_delta; // Mouse movement in the last frame

	void OnKeyPressed(SDL_KeyboardEvent event);
	void OnMouseButtonDown(SDL_MouseButtonEvent event);
	void OnMouseButtonUp(SDL_MouseButtonEvent event);
	void OnMouseMove(SDL_MouseButtonEvent event);
	void OnWheel(SDL_MouseWheelEvent event);
	void OnFileChanged(const char* filename);

	// CPU Global framebuffer
	Image framebuffer;

	// Constructor and main methods
	Application(const char* caption, int width, int height);
	~Application();

	void Init( void );
	void Render( void );
	void Update( float dt );

	// Other methods to control the app
	void SetWindowSize(int width, int height) {
		glViewport( 0,0, width, height );
		this->window_width = width;
		this->window_height = height;
		this->framebuffer.Resize(width, height);
		this->preview_framebuffer.Resize(width, height);
	}

	Vector2 GetWindowSize()
	{
		int w,h;
		SDL_GetWindowSize(window,&w,&h);
		return Vector2(float(w), float(h));
	}
};
