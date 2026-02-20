#pragma once
#include "framework.h"
#include "mesh.h" //per la malla 
#include "camera.h" //camara
#include "image.h" //imatges 
#include "shader.h"	//shaders
#include "texture.h" //textures

class Entity {
public:

	Shader* shader; //material de la entitat (punter a un shader que defineix com es renderitza l'entitat)
	Texture* texture_gpu; //textura de la entitat a la GPU (punter a un objecte Texture que conté la textura carregada a la GPU)
	
	Mesh* mesh; //punter a la malla
	Matrix44 model; //matriu de modelat

	Entity(); //constructor
	Entity(Mesh* mesh); //constructor amb malla

	Image* texture = nullptr;
	
	float time;

	//rendering mode for the entity
	enum class eRenderMode { POINTCLOUD, WIREFRAME, TRIANGLES, TRIANGLES_INTERPOLATED };
	eRenderMode mode = eRenderMode::TRIANGLES_INTERPOLATED;

	//opcions de renderitzat
	bool use_texture = true; //T: utilitzar textura vs color pla
	bool interpolate_uvs = true; //C: utilitzar UVs interpolades vs UVs pla (UV del vèrtex 0 per a tot el triangle) 

	//flat_color blanc per a renderitzat amb color pla (sense textura ni interpolació de color) --> només s'utilitza quan mode == TRIANGLES i interpolate_uvs == false (UVs pla)
	Color flat_color = Color::WHITE;

	//metodes
	//void Render(Image* framebuffer, Camera* camera, const Color& c); //renderitza l'entitat (2.2)
	//void Render(Image* framebuffer, Camera* camera, FloatImage* zbuffer);
	void Update(float dt); //actualitza l'entitat
	void Render(Camera* camera); //nou metode GPU --> renderitza l'entitat utilitzant OpenGL i el shader associat a l'entitat
};