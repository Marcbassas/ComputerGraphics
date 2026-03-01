#pragma once
#include "framework.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"

//STRUCT LLUM 
struct sLight {
	Vector3 position; //posició de la llum en el món
    Vector3 color; //color de la llum 
};

//STRUCT DADES UNIFORMES
struct sUniformData {
    Matrix44 model; //matriu modelat
	Matrix44 viewprojection; //matriu de vista i projecció combinades
	Vector3 ambient_light = Vector3(0.1f, 0.1f, 0.1f); //llum ambient global
	Vector3 camera_position; //posició de la càmera 
	sLight lights[4]; //array de llums
	int num_lights = 1; //nombre de llums actives (de 0 a 4)
};

//classe material
class Material {
public:
    Shader* shader = nullptr; //shader del material 

    //textures
	Texture* color_texture = nullptr; //textura de color (diffuse)
	Texture* specular_texture = nullptr; //textura especular (specular)
	Texture* normal_texture = nullptr; //textura de normals (normal map)

    //propietats material
    Vector3 Ka = Vector3(0.1f, 0.1f, 0.1f); //ambient
    Vector3 Kd = Vector3(0.8f, 0.8f, 0.8f); //difús
    Vector3 Ks = Vector3(0.5f, 0.5f, 0.5f); //especular
    float shininess = 32.0f; //brillantor del material 

    //toggles textures
	bool use_color_texture = true; //utilitzar textura de color vs color pla (Ka, Kd, Ks)
	bool use_specular_texture = false; //utilitzar textura especular vs Ks
	bool use_normal_texture = false; //utilitzar textura de normals vs normals calculats a partir de la geometria

    Material() {}

	void Enable(const sUniformData& data); //activa el material --> configura el shader i les textures per al renderitzat, puja les dades uniformes necessaries al shader
    void Disable();
};