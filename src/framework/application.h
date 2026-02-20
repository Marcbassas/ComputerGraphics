/*  
	+ This class encapsulates the application, is in charge of creating the data, getting the user input, process the update and render.
*/
#pragma once
#include "entity.h" //LAB 2
#include <vector>
#include "main/includes.h"
#include "framework.h"
#include "image.h"
#include "utils.h" //per el random
#include "mesh.h"
#include "shader.h"
#include "texture.h"

//----------------CLASE BOTÓ ----------------
enum ButtonType { 
	BUTTON_PENCIL, //llapis
	BUTTON_COLOR_RED, //color vermell
	BUTTON_COLOR_GREEN, //color verd
	BUTTON_COLOR_BLUE, //color blau
	BUTTON_ERASER, //goma
	BUTTON_TRIANGLE, //triangle
	BUTTON_LINE, //línia
	BUTTON_RECT, //rectangle 
	BUTTON_CLEAR, //netejar
	BUTTON_LOAD, //carregar
	BUTTON_SAVE //desar
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

//----------------CLASE EINES----------------
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
	static const int MAX_PARTICLES = 500; //num de gotes

	struct Particle {
		Vector2 position; //posició
		Vector2 velocity; //velocitat
		Color color; //color de la particula
		float ttl; //vida de la particula
		bool inactive; //si la particula està inactiva
	};
	 
	Particle particles[MAX_PARTICLES]; //array de particules

	//funcio per posar una particula a una posició aleatoria --> random values de utils.h
	void ResetParticle(Particle& p, int width, int height, bool start_at_top) {
		p.position.x = randomValue() * width; //posició x aleatòria

		if (start_at_top) p.position.y = 0.0f; //si ha de començar a dalt
		else p.position.y = randomValue() * height; //posició y aleatòria

		p.velocity = Vector2(0.0f, 150.0f + randomValue() * 200.0f); //caure verticalment amb velocitat aleatòria
		p.color = Color(200, 200, 255); //color blau clar
		p.ttl = 2.0f + randomValue() * 3.0f; //vida aleatòria
		p.inactive = false; //partícula activa 
	}

	void Init(int width, int height) { //inicialitza totes les particules
		for (int i = 0; i < MAX_PARTICLES; i++) { //for per cada particula
			ResetParticle(particles[i], width, height, false); //posa la particula a una posició aleatòria 
		}
	}

	void Update(float dt, int width, int height) { //actualitza totes les particules
		for (int i = 0; i < MAX_PARTICLES; i++) { //for per cada particula
			Particle& p = particles[i]; //referència a la particula actual

			//moure la partícula: posició = posició + velocitat * temps
			p.position = p.position + p.velocity * dt;

			//restar vida a la partícula
			p.ttl -= dt;

			//si surt de la pantalla per baix (Y > height) o s'acaba el temps...
			if (p.position.y > height || p.ttl <= 0) {
				//la regenerem a dalt de tot (true) perquè segueixi plovent
				ResetParticle(p, width, height, true); //es torna a posar a dalt
			}
		}
	}

	void Render(Image* framebuffer) { //dibuixa totes les particules --> framebuffer
		for (int i = 0; i < MAX_PARTICLES; i++) {
			// Només dibuixem si està dins la pantalla
			if (particles[i].position.x >= 0 && particles[i].position.x < framebuffer->width && particles[i].position.y >= 0 && particles[i].position.y < framebuffer->height) { //comprova si està dins la pantalla
				framebuffer->SetPixel((int)particles[i].position.x, (int)particles[i].position.y, particles[i].color); //dibuixa la partícula
			}
		}
	}
}; 


//----------------CLASSE APPLICATION ----------------
class Application
{
public:

	int current_mode = 0;   // 0 = 3D single, 1 = quad, 5 = multi, etc.
	int quad_mode = 0;      // 0..5 = a..f

	Mesh* quad_mesh = nullptr;
	Shader* quad_shader = nullptr;
	Texture* quad_texture = nullptr;

	//variables globals per a l'eina actual, color, amplada del contorn i si omplir formes
	std::vector<Button> buttons;

	ParticleSystem rain; //2.3
	ToolType current_tool = TOOL_PENCIL; // Eina per defecte
	Color current_color = Color::BLACK;  // Color per defecte

    //modes
	bool fill_shapes = false;  // F = toggle
	int border_width = 1;      // + / -

	//framebuffers
	Image preview_framebuffer; //framebuffer de previsualització
	Image framebuffer;

	// Tool state
	bool is_drawing = false; //si esta dibuxant
	Vector2 start_pos; //posició inicial del dibuix
	Vector2 current_pos; //posició actual del dibuix

	// Window
	SDL_Window* window = nullptr; 
	int window_width; //amplada finestra
	int window_height; //altura finestra

	float time;

	// Input
	const Uint8* keystate;
	int mouse_state; // Tells which buttons are pressed
	Vector2 mouse_position; // Last mouse position
	Vector2 mouse_delta; // Mouse movement in the last frame

	// last raw mouse position used by OnMouseMove to compute deltas
	int last_mouse_x = 0;
	int last_mouse_y = 0;

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
		// Actualitzar aspect ratio de la càmera per evitar deformacions en redimensionar
		if (camera) {
			// Recalcular la perspectiva amb el nou aspect ratio
			camera->SetPerspective(camera->fov, width / (float)height, camera->near_plane, camera->far_plane);
		}
	}

	Vector2 GetWindowSize()
	{
		int w,h;
		SDL_GetWindowSize(window,&w,&h);
		return Vector2(float(w), float(h));
	}

	// 2. Declara la càmera (molt important, t'està donant error perquè falta)
	Camera* camera;

	// 2.5 -> quina propietat de la camera estem modificant (NEAR/FAR/FOV)
	enum CameraProperty { CAM_NONE = -1, CAM_NEAR = 0, CAM_FAR = 1, CAM_FOV = 2 };
	CameraProperty current_camera_property = CAM_NONE; // per defecte no modifiquem cap propietat

	// 3. El vector d'entitats
	std::vector<Entity*> entities;

	//renderitzar les oclusions (mode 1 i mode 2)
	bool use_occlusions = true; //z buffer per defecte activat
};
