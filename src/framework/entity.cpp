#include "entity.h"
#include <cmath> // Necessari per sin i cos
#include <map>   // ✅ AFEGEIX AQUESTA LÍNIA


Entity::Entity(){
    model.SetIdentity();
	//Inicialitzar els valors per a l'animació
	this->mesh = nullptr; // no té malla per defecte
	texture = nullptr;
	this->time = 0.0f;    //el temps comença a 0, i s'anirà acumulant a mesura que passi el temps en Update
}

Entity::Entity(Mesh* mesh) { //constructor amb malla
	this->mesh = mesh; //punter a la malla
	model.SetIdentity();
	texture = nullptr;
	this->time = 0.0f; //temps = 0 per començar l'animació des del principi
}

//actualittza l'aplicacio en funcio del temps que ha passat
void Entity::Update(float seconds_elapsed) {
	this->time += seconds_elapsed;

	float rotation_speed = 2.0f;          //rotació contínua
	float animation_speed = this->time * 3.0f; //velocitat de l'animació oscil·lant (pulsació i reboteig) --> multipliquem el temps per un factor per controlar la velocitat de l'oscil·lació

	//escala base 5.0f + petita pulsació
	float s = 5.0f + sin(animation_speed) * 0.5f;   //oscil·la entre ~4.5 i 5.5

	//Y que rebota
	float y_pos = sin(animation_speed) * 2.5f;

	//recuperar la x inicial per mantenir les entitats separades i que no es moguin horitzontalment
	static std::map<Entity*, float> initial_x_positions; //posicions x inicials de les entitats --> punter a l'entitat
	if (initial_x_positions.find(this) == initial_x_positions.end()) { //si no hem guardat pos inicial --> és la primera vegada que actualitzem aquesta entitat
		initial_x_positions[this] = model.m[12];
	}
	float initial_x = initial_x_positions[this]; //x inicial guardada per a aquesta entitat --> no canvia amb el temps

	//matriuys de transformació
	Matrix44 m_scale;    m_scale.MakeScaleMatrix(s, s, s); //matriu de escalat amb oscilacio
	Matrix44 m_rot_y;    m_rot_y.MakeRotationMatrix(this->time * rotation_speed, Vector3(0, 1, 0)); //matriu de rotacio per fer girar l'entitat al voltant de eix Y 
	Matrix44 m_rot_x;    m_rot_x.MakeRotationMatrix(3.14159f, Vector3(1, 0, 0));  //rotació per posar-lo dret
	Matrix44 m_trans;    m_trans.MakeTranslationMatrix(initial_x, y_pos, 0.0f); //matriu de translació que manté la x inicial i fa que l'entitat reboti en y

	//Translation * RotY * RotX * Scale
	model = m_trans * m_rot_y * m_rot_x * m_scale;
}

/*
void Entity::Render(Image* framebuffer, Camera* camera, const Color& c) {
	if (!mesh) return;

	const std::vector<Vector3>& vertices = mesh->GetVertices();

	auto inside_clip = [](const Vector3& p)->bool {
		return p.x >= -1.0f && p.x <= 1.0f &&
			p.y >= -1.0f && p.y <= 1.0f &&
			p.z >= -1.0f && p.z <= 1.0f;
		};

	// Iterar sobre els vèrtexs de 3 en 3 (triangles)
	for (size_t i = 0; i + 2 < vertices.size(); i += 3) {
		// Transformar LOCAL → WORLD
		Vector3 w0 = model * vertices[i];
		Vector3 w1 = model * vertices[i + 1];
		Vector3 w2 = model * vertices[i + 2];

		// Transformar WORLD → CLIP SPACE
		Vector3 p0 = camera->ProjectVector(w0);
		Vector3 p1 = camera->ProjectVector(w1);
		Vector3 p2 = camera->ProjectVector(w2);

		// Rebutjar triangles fora del frustum
		if (!inside_clip(p0) || !inside_clip(p1) || !inside_clip(p2))
			continue;

		// Transformar CLIP SPACE → SCREEN SPACE
		Vector3 s0 = p0;
		Vector3 s1 = p1;
		Vector3 s2 = p2;

		s0.x = (s0.x + 1.0f) * 0.5f * framebuffer->width;
		s0.y = (1.0f - (s0.y + 1.0f) * 0.5f) * framebuffer->height;

		s1.x = (s1.x + 1.0f) * 0.5f * framebuffer->width;
		s1.y = (1.0f - (s1.y + 1.0f) * 0.5f) * framebuffer->height;

		s2.x = (s2.x + 1.0f) * 0.5f * framebuffer->width;
		s2.y = (1.0f - (s2.y + 1.0f) * 0.5f) * framebuffer->height;

		// MANTENIR com Vector3 (la funció els rep així)
		// Coordenades baricèntriques: cada vèrtex amb el seu color
		Color c0 = Color(255, 0, 0);   // Vermell
		Color c1 = Color(0, 255, 0);   // Verd
		Color c2 = Color(0, 0, 255);   // Blau

		// Dibuixar triangle interpolat amb Vector3
		framebuffer->DrawTriangleInterpolated(s0, s1, s2, c0, c1, c2);
	}
}
*/

void Entity::Render(Image* framebuffer, Camera* camera, FloatImage* zbuffer) {
	if (!mesh) return;

	const auto& vertices = mesh->GetVertices(); //punter a la llista de vèrtexs de la malla
	const auto& uvs = mesh->GetUVs(); //punter a la llista de coordenades UV de la malla

	auto inside_clip = [](const Vector3& p)->bool { //funcio per comprovar si un punt està dins del frustum de càmera (clip space)
		return p.x >= -1 && p.x <= 1 &&
			p.y >= -1 && p.y <= 1 &&
			p.z >= -1 && p.z <= 1;
		};

	for (size_t i = 0; i + 2 < vertices.size(); i += 3) //iterar sobre els vèrtexs de 3 en 3 (triangles)
	{
		// Transformar LOCAL → WORLD
		Vector3 w0 = model * vertices[i]; 
		Vector3 w1 = model * vertices[i + 1]; 
		Vector3 w2 = model * vertices[i + 2]; 

		// Transformar WORLD → CLIP SPACE
		Vector3 p0 = camera->ProjectVector(w0);
		Vector3 p1 = camera->ProjectVector(w1);
		Vector3 p2 = camera->ProjectVector(w2);

		if (!inside_clip(p0) || !inside_clip(p1) || !inside_clip(p2)) //si algun vèrtex està fora del frustum --> rebutjar el triangle
			continue;
		//Transformar CLIP SPACE → SCREEN SPACE
		Vector3 s0 = p0; 
		Vector3 s1 = p1; 
		Vector3 s2 = p2;

		//formula de transformació a coordenades de pantalla: x = (vector x + 1) * 0.5f * framebuffer width, y = (1 - (vector y + 1) * 0.5f) * framebuffer height
		s0.x = (s0.x + 1) * 0.5f * framebuffer->width;
		s0.y = (1 - (s0.y + 1) * 0.5f) * framebuffer->height;

		s1.x = (s1.x + 1) * 0.5f * framebuffer->width;
		s1.y = (1 - (s1.y + 1) * 0.5f) * framebuffer->height;

		s2.x = (s2.x + 1) * 0.5f * framebuffer->width;
		s2.y = (1 - (s2.y + 1) * 0.5f) * framebuffer->height;

		//dibuixar triangle interpolat amb menys paràmetres
		sTriangleInfo info; //info = estruct quue conté tota la informació necessària per dibuixar el triangle (vèrtexs, coordenades UV, colors, textura)
        info.v[0] = s0; info.v[1] = s1; info.v[2] = s2;
        info.uv[0] = uvs[i]; info.uv[1] = uvs[i + 1]; info.uv[2] = uvs[i + 2];
        info.c[0] = Color::WHITE; info.c[1] = Color::WHITE; info.c[2] = Color::WHITE;
        info.texture = texture;

		framebuffer->DrawTriangleInterpolated(info, zbuffer); //dibuixar triangle amb interpolació de color i textura, passant tota la informació del triangle a través de l'estructura sTriangleInfo
	}
}

