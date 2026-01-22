/*  
	+ This class encapsulates the application, is in charge of creating the data, getting the user input, process the update and render.
*/
#pragma once

#include "main/includes.h"
#include "framework.h"
#include "image.h"
#include "utils.h" //Per la aletorietat
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

//---------------- CLASE DE PARTICULES ---------------
class ParticleSystem {
public:
	static const int MAX_PARTICLES = 500; // Nombre de gotes

	struct Particle {
		Vector2 position;
		Vector2 velocity;
		Color color;
		float ttl;      // Vida de la particula
		bool inactive;
	};

	Particle particles[MAX_PARTICLES];

	// Aquesta funció posa una partícula a punt
	void ResetParticle(Particle& p, int width, int height, bool start_at_top) {
		p.position.x = randomValue() * width; // Posició X aleatòria (usa utils.h)

		if (start_at_top) p.position.y = 0.0f; // Si plou, comença a dalt
		else p.position.y = randomValue() * height; // A l'inici, per tot arreu

		p.velocity = Vector2(0.0f, 150.0f + randomValue() * 200.0f); // Cau cap avall
		p.color = Color(200, 200, 255); // Color blauet
		p.ttl = 2.0f + randomValue() * 3.0f; // Vida aleatòria
		p.inactive = false;
	}

	void Init(int width, int height) {
		for (int i = 0; i < MAX_PARTICLES; i++) {
			ResetParticle(particles[i], width, height, false);
		}
	}

	void Update(float dt, int width, int height) {
		for (int i = 0; i < MAX_PARTICLES; i++) {
			Particle& p = particles[i]; // Referència per escriure menys

			// 1. Moure la partícula: Posició = Posició + Velocitat * temps
			p.position = p.position + p.velocity * dt;

			// 2. Restar vida
			p.ttl -= dt;

			// 3. Si surt de la pantalla per baix (Y > height) o s'acaba el temps...
			if (p.position.y > height || p.ttl <= 0) {
				// La regenerem a dalt de tot (true) perquè segueixi plovent
				ResetParticle(p, width, height, true);
			}
		}
	}

	void Render(Image* framebuffer) {
		for (int i = 0; i < MAX_PARTICLES; i++) {
			// Només dibuixem si està dins la pantalla
			if (particles[i].position.x >= 0 && particles[i].position.x < framebuffer->width &&
				particles[i].position.y >= 0 && particles[i].position.y < framebuffer->height) {

				// Dibuixem un puntet de color blau
				framebuffer->SetPixel((int)particles[i].position.x, (int)particles[i].position.y, particles[i].color);
			}
		}
	}
}; // Final de la classe ParticleSystem

//---------------- APPLICATION CLASS ----------------
class Application
{
public:
	//variables globals per a l'eina actual, color, amplada del contorn i si omplir formes
	std::vector<Button> buttons;

	ParticleSystem rain; //2.3
	ToolType current_tool = TOOL_PENCIL; // Eina per defecte
	Color current_color = Color::BLACK;  // Color per defecte

	//modes
	int current_mode = 1;      // 1 = Paint, 2 = Animation
	bool fill_shapes = false;  // F = toggle
	int border_width = 1;      // + / -

	//framebuffers
	Image preview_framebuffer; //framebuffer de previsualització
	Image framebuffer;

	// Tool state
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
