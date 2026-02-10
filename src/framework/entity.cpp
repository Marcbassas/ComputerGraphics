#include "entity.h"
#include <cmath> // Necessari per sin i cos
#include <map>   // ✅ AFEGEIX AQUESTA LÍNIA


Entity::Entity(){
    model.SetIdentity();
	//Inicialitzar els valors per a l'animació
	this->mesh = nullptr; // no té malla per defecte
	this->time = 0.0f;    //el temps comença a 0, i s'anirà acumulant a mesura que passi el temps en Update
}

Entity::Entity(Mesh* mesh) { //constructor amb malla
	this->mesh = mesh; //punter a la malla
	model.SetIdentity();
	this->time = 0.0f; //temps = 0 per començar l'animació des del principi
}

//actualittza l'aplicacio en funcio del temps que ha passat
void Entity::Update(float seconds_elapsed) {
	this->time += seconds_elapsed;

	float rotation_speed = 2.0f;          // rotació contínua
	float animation_speed = this->time * 3.0f;

	// Escala base 5.0f + petita pulsació
	float s = 5.0f + sin(animation_speed) * 0.5f;   // oscil·la entre ~4.5 i 5.5

	// Posició Y que rebota
	float y_pos = sin(animation_speed) * 2.5f;

	// Recuperem X inicial (com ja tenies)
	static std::map<Entity*, float> initial_x_positions;
	if (initial_x_positions.find(this) == initial_x_positions.end()) {
		initial_x_positions[this] = model.m[12];
	}
	float initial_x = initial_x_positions[this];

	// Matrius
	Matrix44 m_scale;    m_scale.MakeScaleMatrix(s, s, s);
	Matrix44 m_rot_y;    m_rot_y.MakeRotationMatrix(this->time * rotation_speed, Vector3(0, 1, 0));
	Matrix44 m_rot_x;    m_rot_x.MakeRotationMatrix(3.14159f, Vector3(1, 0, 0));  // <-- IMPORTANT: la rotació per posar-lo dret
	Matrix44 m_trans;    m_trans.MakeTranslationMatrix(initial_x, y_pos, 0.0f);

	// Ordre correcte: Translation * RotY * RotX * Scale
	model = m_trans * m_rot_y * m_rot_x * m_scale;
}

void Entity::Render(Image* framebuffer, Camera* camera, const Color& c) {
	if (!mesh) return; //si no hi ha malla, no renderitzem res

	const std::vector<Vector3>& vertices = mesh->GetVertices(); //vector de vertexs de la malla

	auto inside_clip = [](const Vector3& p)->bool { //funció lambda per a saber si un punt està dins del cub de retallat
		return p.x >= -1.0f && p.x <= 1.0f &&
			   p.y >= -1.0f && p.y <= 1.0f &&
			   p.z >= -1.0f && p.z <= 1.0f;
	};

	//iterar sobre els vertexs de la malla de 3 en 3 (triangles) (2.2.1)
	for (size_t i = 0; i + 2 < vertices.size(); i += 3) { //size_t perque és un índex, recorre tots els triangles (3 vertexs per triangle)
		////transformar els vertexs de LOCAL --> WORLD
		//multipliquem la matriu de modelat per cada vertex del triangle
		Vector3 w0 = model * vertices[i];
		Vector3 w1 = model * vertices[i + 1];
		Vector3 w2 = model * vertices[i + 2];

		//transformar els vertexs de WORLD --> CAMERA(CLIP SPACE)
		//projectar els vertexs de 3D a 2D (homogeneous space)
		Vector3 p0 = camera->ProjectVector(w0);
		Vector3 p1 = camera->ProjectVector(w1);
		Vector3 p2 = camera->ProjectVector(w2);

		//rebutjar triangles fora del cub de retallat (-1 a 1 en x,y,z)
		if (!inside_clip(p0) || !inside_clip(p1) || !inside_clip(p2))
			continue;

		//transformar els vertexs de CLIP SPACE --> FRAMEBUFFER
		//camera= x(de 1 a -1) --> framebuffer = x(de 0 a width)
		//transormar coordenades pantalla al component x: vector x = (vector x + 1) * 0.5f * framebuffer width
		//transformar coordenadees pantalla al component y: vector y = (1 - (vector y + 1) * 0.5f) * framebuffer height
		Vector3 s0 = p0; //vectors s --> screen space
		Vector3 s1 = p1;
		Vector3 s2 = p2;

		s0.x = (s0.x + 1.0f) * 0.5f * framebuffer->width;
		s0.y = (1.0f - (s0.y + 1.0f) * 0.5f) * framebuffer->height;

		s1.x = (s1.x + 1.0f) * 0.5f * framebuffer->width;
		s1.y = (1.0f - (s1.y + 1.0f) * 0.5f) * framebuffer->height;

		s2.x = (s2.x + 1.0f) * 0.5f * framebuffer->width;
		s2.y = (1.0f - (s2.y + 1.0f) * 0.5f) * framebuffer->height;

        // LAB3: 3.1: dibuixar triangle omplert en comptes de wireframe
        //convertim a Vector2 i cridem a la funció de dibuixar triangle (omplert)
        Vector2 tv0((int)s0.x, (int)s0.y); 
        Vector2 tv1((int)s1.x, (int)s1.y);
        Vector2 tv2((int)s2.x, (int)s2.y);
        //dibuixar triangle amb el mateix color per contorn i omplert
        framebuffer->DrawTriangle(tv0, tv1, tv2, c, true, c);
	}
}
