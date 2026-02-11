#pragma once
#include "framework.h"
#include "mesh.h" //per la malla 
#include "camera.h" //camara
#include "image.h" //imatges 

class Entity {
public:
	
	Mesh* mesh; //punter a la malla
	Matrix44 model; //matriu de modelat

	Entity(); //constructor
	Entity(Mesh* mesh); //constructor amb malla

	Image* texture = nullptr;
	
	float time;

	// rendering mode for the entity
	enum class eRenderMode { POINTCLOUD, WIREFRAME, TRIANGLES, TRIANGLES_INTERPOLATED };
	eRenderMode mode = eRenderMode::TRIANGLES_INTERPOLATED;

	// toggles
	bool use_texture = true; // T: use mesh texture vs color per vertex
	bool interpolate_uvs = true; // C: interpolated UVs vs plain color

	//metodes
	//void Render(Image* framebuffer, Camera* camera, const Color& c); //renderitza l'entitat (2.2)
	void Render(Image* framebuffer, Camera* camera, FloatImage* zbuffer);
	void Update(float dt); //actualitza l'entitat
};