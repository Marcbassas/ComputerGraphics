#include "entity.h"

Entity::Entity(){
	model.SetIdentity();
}

Entity::Entity(Mesh* mesh){
	this->mesh = mesh;
	model.SetIdentity();
}

void Entity :: Update(float dt){
	//per ara no fa res
}