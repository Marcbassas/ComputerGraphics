#include "entity.h"
#include <cmath> // Necessari per sin i cos

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

void Entity::Update(float seconds_elapsed) {
	this->time += seconds_elapsed; //acumular el temps

	float rotation_speed = 2.0f; //velocitat de gir
	float angle = this->time * rotation_speed; //angle = temps acumulat * velocitat de gir --> l'angle va creixent constantment, fent que l'entitat giri contínuament

	float animation_speed = this->time * 3.0f; //velocitat del batec de l'animació (com més gran, més ràpid)
	float s = 1.0f + sin(animation_speed) * 0.1f; //factor d'escala
	float y_pos = sin(animation_speed) * 2.0f; //posicio vertical (batec) --> va pujant i baixant amb una amplitud de 2.0f

	//MATRIUS
	Matrix44 m_scale; //matriu escala
	m_scale.MakeScaleMatrix(s, s, s); //aplicar el factor d'escala a la matriu d'escala

	Matrix44 m_rotation; //matriu de rotació
	m_rotation.MakeRotationMatrix(angle, Vector3(0, 1, 0)); //aplicar la rotació al voltant de l'eix Y amb l'angle calculat (que fa que giri contínuament)

	Matrix44 m_fix; //matriu de correcció de l'eix
	m_fix.MakeRotationMatrix(3.14159f, Vector3(0, 0, 1)); //correcio del eix de rotacio 

	float current_x = model.m[12]; //posició actual en x de l'entitat (obtenim la component de translació de la matriu de modelat) --> per mantenir la posició horizontal fixa mentre l'entitat puja i baixa amb el batec
	Matrix44 m_translation; //matriu de translació
	m_translation.MakeTranslationMatrix(current_x, y_pos, 0.0f); //aplicar la translació amb la posición horizontal fija (current_x) y la posición vertical variable (y_pos) para el efecte del batec

	//COMBINACIO DE LES MATRIUS: Model = Translation * Rotation * Fix * Scale (T * R * F * S)S
	model = m_translation * m_rotation * m_fix * m_scale;
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

		//dibuixar el triangle a la framebuffer (només les vores) --> DrawLineDDA --> wireframe (dibuixar les línies entre els 3 punts)
		framebuffer->DrawLineDDA((int)s0.x, (int)s0.y, (int)s1.x, (int)s1.y, c);
		framebuffer->DrawLineDDA((int)s1.x, (int)s1.y, (int)s2.x, (int)s2.y, c);
		framebuffer->DrawLineDDA((int)s2.x, (int)s2.y, (int)s0.x, (int)s0.y, c);
	}
}
