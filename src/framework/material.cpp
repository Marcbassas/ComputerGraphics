#include "material.h"

void Material::Enable(const sUniformData& data) //fuincio per activar el material (configura el shader)
{
	if (!shader) return; //si no hi ha shader --> no s'activa el material
	shader->Enable(); //activa el shader

    //MATRIUS 
	shader->SetMatrix44("u_model", data.model); //matriu de modelat
	shader->SetMatrix44("u_viewprojection", data.viewprojection);//matriu de vista i projecció combinades

    //CAMERA
	shader->SetVector3("u_camera_position", data.camera_position); //posició de la càmera

    //Llum ambient
	shader->SetVector3("u_ambient_light", data.ambient_light); //llum ambient global

    //Llum (primera)
	shader->SetVector3("u_light_position", data.lights[0].position); //posició de la llum
	shader->SetVector3("u_light_color", data.lights[0].color); //color de la llum

    //Propietats material
	shader->SetVector3("u_Ka", Ka); //ambient
	shader->SetVector3("u_Kd", Kd); //difús
	shader->SetVector3("u_Ks", Ks); //especular
	shader->SetUniform1("u_shininess", shininess); //brillantor del material

    //textures
	shader->SetUniform1("u_use_color_texture", (int)use_color_texture); //textura del color vs color pla
	shader->SetUniform1("u_use_specular_texture", (int)use_specular_texture); //textura especular vs Ks
	shader->SetUniform1("u_use_normal_texture", (int)use_normal_texture); //textura de normals vs normals calculats a partir de la geometria

	if (color_texture) //si hi ha textura de color --> la puja al shader
		shader->SetTexture("u_color_texture", color_texture); //textura de color (diffuse)
	if (specular_texture) //si hi ha textura especular --> la puja al shader
		shader->SetTexture("u_specular_texture", specular_texture); //textura especular (specular)
	if (normal_texture) //si hi ha textura de normals --> la puja al shader
		shader->SetTexture("u_normal_texture", normal_texture); //textura de normals (normal map)
}

void Material::Disable() //funcio per desactivar el material (desactiva el shader)
{
	if (shader) shader->Disable(); //si hi ha shader --> desactiva el shader
}