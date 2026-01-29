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

	//metodes
	void Render(Image* framebuffer, Camera* camera, const Color& c); //renderitza l'entitat (2.2)
	void Update(float dt); //actualitza l'entitat
};