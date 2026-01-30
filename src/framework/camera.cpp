#include "camera.h"

#include "main/includes.h"
#include <iostream>

Camera::Camera()
{
	view_matrix.SetIdentity();
	SetOrthographic(-1,1,1,-1,-1,1);
}

Vector3 Camera::GetLocalVector(const Vector3& v)
{
	Matrix44 iV = view_matrix;
	if (iV.Inverse() == false)
		std::cout << "Matrix Inverse error" << std::endl;
	Vector3 result = iV.RotateVector(v);
	return result;
}

Vector3 Camera::ProjectVector(Vector3 pos)
{
	Vector4 pos4 = Vector4(pos.x, pos.y, pos.z, 1.0);
	Vector4 result = viewprojection_matrix * pos4;
	if (type == ORTHOGRAPHIC)
		return result.GetVector3();
	else
		return result.GetVector3() / result.w;
}

void Camera::Rotate(float angle, const Vector3& axis)
{
	Matrix44 R;
	R.MakeRotationMatrix(angle, axis);
	Vector3 new_front = R * (center - eye);
	center = eye + new_front;
	UpdateViewMatrix();
}

void Camera::Move(Vector3 delta)
{
	Vector3 localDelta = GetLocalVector(delta);
	eye = eye - localDelta;
	center = center - localDelta;
	UpdateViewMatrix();
}

void Camera::SetOrthographic(float left, float right, float top, float bottom, float near_plane, float far_plane)
{
	type = ORTHOGRAPHIC;

	this->left = left;
	this->right = right;
	this->top = top;
	this->bottom = bottom;
	this->near_plane = near_plane;
	this->far_plane = far_plane;

	UpdateProjectionMatrix();
}

void Camera::SetPerspective(float fov, float aspect, float near_plane, float far_plane)
{
	type = PERSPECTIVE;

	this->fov = fov;
	this->aspect = aspect;
	this->near_plane = near_plane;
	this->far_plane = far_plane;

	UpdateProjectionMatrix();
}

void Camera::LookAt(const Vector3& eye, const Vector3& center, const Vector3& up)
{
	this->eye = eye;
	this->center = center;
	this->up = up;

	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	// Reset Matrix (Identity)
	view_matrix.SetIdentity();

	// Comment this line to create your own projection matrix!
	//SetExampleViewMatrix();

	//bases de la camera 
	//camera: eye --> posicio de la camera, center --> on mira, up --> vector vertical de la camara
	Vector3 z = (eye - center).Normalize(); //endevant (foward) 
	Vector3 x = up.Cross(z).Normalize(); //dreta --> funcio cross que fa el producte vectorial (up x z) 
	Vector3 y = z.Cross(x).Normalize(); //adalt --> producte vectorial (z x x)

	//matriu de rotacio --> canvia de base
	//eixos de la camara expressats en coordenades del mon
	//converteix de coordenades del mon a coordenades de la camara (canvi de base)
	/*
	R =
	| x.x   y.x   z.x   0 |
	| x.y   y.y   z.y   0 |
	| x.z   y.z   z.z   0 |
	|  0     0     0    1 |

	*/

	//matriu de translacio --> mou l'escena en sentit contrari a la camara
	//matriu que trasllada el mon perque la camara quedi a l'origen
	//si camera = (eye.x, eye.y, eye.z) --> traslladem el mon = (-eye.x, -eye.y, -eye.z)
	/*
	T =
	| 1   0   0   -eye.x |
	| 0   1   0   -eye.y |
	| 0   0   1   -eye.z |
	| 0   0   0     1    |

	*/

	//matriu final = rotacio * translacio --> view matrix
	//tres primeres columnes --> eixos de la camara
	//ultima columna --> traslacio
	/*
	| x.x  y.x  z.x  -x·eye |
	| x.y  y.y  z.y  -y·eye |
	| x.z  y.z  z.z  -z·eye |
	| 0    0    0      1    |
	*/

	//column major  
	view_matrix.m[0] = x.x;	view_matrix.m[4] = y.x;	view_matrix.m[8] = z.x;	view_matrix.m[12] = -x.Dot(eye); //eix x de la camera
	view_matrix.m[1] = x.y;	view_matrix.m[5] = y.y;	view_matrix.m[9] = z.y;	view_matrix.m[13] = -y.Dot(eye); //eix y de la camera
	view_matrix.m[2] = x.z;	view_matrix.m[6] = y.z;	view_matrix.m[10] = z.z;	view_matrix.m[14] = -z.Dot(eye); //eix z de la camera
	view_matrix.m[3] = 0.0f;	view_matrix.m[7] = 0.0f;	view_matrix.m[11] = 0.0f;	view_matrix.m[15] = 1.0f; //traslacio

	UpdateViewProjectionMatrix(); //per actualitzar la viewprojection matrix 
}

// Create a projection matrix
void Camera::UpdateProjectionMatrix()
{
	// Reset Matrix (Identity)
	projection_matrix.SetIdentity();

	// Comment this line to create your own projection matrix!
	//SetExampleProjectionMatrix();

	// Remember how to fill a Matrix4x4 (check framework slides)
	
	if (type == PERSPECTIVE){ //convertir el fov a radians
		float fov_rad = fov * (3.14159265f / 180.0f); //conversio a radians
		float f = 1.0f / tan(fov_rad / 2.0f); //factor d'escalat (zoom)

		//column major --> omplir la matriu de projeccio
		//x: f /aspect
		//y: f
		//matriu
		//primera ccolumna --> controla la escala en x (f/aspect)
		//segona columna --> controla la escala en y (f)
		//tercera columna --> controla la profunditat (z)
		//quarta columna --> perspectiva
		/*
		| f/aspect  0		0				0 |
		| 0			f		0				0 |
		| 0			0		(f+n)/(n-f)	   -1 |
		| 0			0		(2fn)/(n-f)		0 |
		
		*/
		projection_matrix.m[0] = f / aspect; //aspect ratio
		projection_matrix.m[1] = 0; 
		projection_matrix.m[2] = 0; 
		projection_matrix.m[3] = 0;

		projection_matrix.m[4] = 0;
		projection_matrix.m[5] = f; //fov vertical
		projection_matrix.m[6] = 0;
		projection_matrix.m[7] = 0;

		//profuncitat i perspectiva
		//m[10] i m[14] --> transformacio en z 
		//m[11] = -1 i m[15] = 0 per perspectiva

		projection_matrix.m[8] = 0;
		projection_matrix.m[9] = 0;
		projection_matrix.m[10] = (far_plane + near_plane) / (near_plane - far_plane); 
		projection_matrix.m[11] = -1.0f;

		projection_matrix.m[12] = 0;
		projection_matrix.m[13] = 0;
		projection_matrix.m[14] = (2.0f * far_plane * near_plane) / (near_plane - far_plane);
		projection_matrix.m[15] = 0.0f;

	}
	else if (type == ORTHOGRAPHIC){ //matriu orthogonal 
		//els objectes no es fan petits amb la distancia
		//les linies paraleles es mantenen paraleles
		float l = left;
		float r = right;
		float t = top; 
		float b = bottom; 
		float n = near_plane; //near --> profunditat minima
		float f = far_plane; //far --> profunditat maxima

		//convertir de [l,r] a [-1,1] --> x
		//convertir de [b,t] a [-1,1] --> y
		//convertir de [n,f] a [-1,1] --> z
		//openGl te el eix Z cap endavant negatiu

		//ortographic = escala + traslacio
			/*escala:
			x: 2 / (r - l)
			y: 2/(t-b)
			z: -2/(f-n)

			translacio:
			x: -(r+l)/(r-l)
			y: -(t+b)/(t-b)
			z: -(f+n)/(f-n)
		
		matriu:
		| 2/(r-l)				0					0					0 |
		|   0					2/(t-b)				0					0 |
		|   0					0					-2/(f-n)			0 |
		| -(r+l)/(r-l)			-(t+b)/(t-b)		-(f+n)/(f-n)		1 |
		*/

		projection_matrix.m[0] = 2.0f / (r - l);
		projection_matrix.m[1] = 0;
		projection_matrix.m[2] = 0;
		projection_matrix.m[3] = 0;

		projection_matrix.m[4] = 0;
		projection_matrix.m[5] = 2.0f / (t - b);
		projection_matrix.m[6] = 0;
		projection_matrix.m[7] = 0;

		projection_matrix.m[8] = 0;
		projection_matrix.m[9] = 0;
		projection_matrix.m[10] = -2.0f / (f - n);
		projection_matrix.m[11] = 0;

		projection_matrix.m[12] = -(r + l) / (r - l);
		projection_matrix.m[13] = -(t + b) / (t - b);
		projection_matrix.m[14] = -(f + n) / (f - n);
		projection_matrix.m[15] = 1.0f;
	} 

	UpdateViewProjectionMatrix(); //per actualitzar la viewprojection matrix
}

void Camera::UpdateViewProjectionMatrix()
{
	viewprojection_matrix = projection_matrix * view_matrix;
}

Matrix44 Camera::GetViewProjectionMatrix()
{
	UpdateViewMatrix();
	UpdateProjectionMatrix();

	return viewprojection_matrix;
}

// The following methods have been created for testing.
// Do not modify them.

void Camera::SetExampleViewMatrix()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, up.x, up.y, up.z);
	glGetFloatv(GL_MODELVIEW_MATRIX, view_matrix.m );
}

void Camera::SetExampleProjectionMatrix()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (type == PERSPECTIVE)
		gluPerspective(fov, aspect, near_plane, far_plane);
	else
		glOrtho(left,right,bottom,top,near_plane,far_plane);

	glGetFloatv(GL_PROJECTION_MATRIX, projection_matrix.m );
	glMatrixMode(GL_MODELVIEW);
}
