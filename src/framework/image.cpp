#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "GL/glew.h"
#include "../extra/picopng.h"
#include "image.h"
#include "utils.h"
#include "camera.h"
#include "mesh.h"

Image::Image() {
	width = 0; height = 0;
	pixels = NULL;
}

/*
//dibuixar un triangle amb interpolació de color per vèrtexs (LAB3: 3.2)
void Image::DrawTriangleInterpolated(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Color& c0, const Color& c1, const Color& c2) {
	//calcular el bounding box del triangle per limitar el nombre de píxels a processar
	//bounding box = rectangle mínim que conté el triangle, definit per les coordenades mínimes i màximes dels vèrtexs del triangle
	int minX = (int)floor(std::min(std::min(p0.x, p1.x), p2.x)); //coordenada MIN en x del triangle --> calculada com el mínim entre les coordenades x dels 3 vèrtexs
	int minY = (int)floor(std::min(std::min(p0.y, p1.y), p2.y)); //coordenada MIN en y del triangle --> calculada com el mínim entre las coordenades y dels 3 vèrtexs
	int maxX = (int)ceil(std::max(std::max(p0.x, p1.x), p2.x)); //coordenada MAX en x del triangle --> calculada como el máximo entre las coordenades x dels 3 vèrtexs
	int maxY = (int)ceil(std::max(std::max(p0.y, p1.y), p2.y)); //coordenada MAX en y del triangle --> calculada como el máximo entre las coordenades y dels 3 vèrtexs

	//
	if (minX < 0) minX = 0; //coordenades pantalla -->  x: vector x = (vector x + 1) * 0.5f * framebuffer width
	if (minY < 0) minY = 0; //coordenades pantalla -->  y: vector y = (1 - (vector y + 1) * 0.5f) * framebuffer height
	if (maxX >= (int)width)  maxX = (int)width - 1; //coordenades pantalla -->  x: vector x = (vector x + 1) * 0.5f * framebuffer width
	if (maxY >= (int)height) maxY = (int)height - 1; //coordenades pantalla -->  y: vector y = (1 - (vector y + 1) * 0.5f) * framebuffer height

	//calcular el denominador de les coordenades baricèntriques per evitar calcular-lo per cada píxel
	//denominador = (p1.y - p2.y) * (p0.x - p2.x) + (p2.x - p1.x) * (p0.y - p2.y)
	//el denominador es el mateix per tots els píxels del triangle,només depèn de les coordenades dels vèrtexs del triangle
    float denom = ( (p1.y - p2.y) * (p0.x - p2.x) + (p2.x - p1.x) * (p0.y - p2.y) );
	if (fabs(denom) < 1e-6f) return; //si el denom petit --> triangle molt petit o degenerate --> no dibuixem res

	const float eps = 1e-5f; // tolerància per a considerar un píxel com a dins del triangle

	//per cada pixel dins del bounding box, calcular les coordenades baricèntriques i interpolar el color
	for (int y = minY; y <= maxY; ++y) { //per cada coordenada y dins del bounding box
		for (int x = minX; x <= maxX; ++x) { //per cada coordenada x dins del bounding box
			//calcular les coordenades baricèntriques del píxel (x,y) respecte al triangle definit pels vèrtexs p0,p1,p2
            float px = x + 0.5f; //centre del pixel (x,y)
            float py = y + 0.5f; 

			float w0 = ((p1.y - p2.y) * (px - p2.x) + (p2.x - p1.x) * (py - p2.y)) / denom; //coordenada baricèntrica respecte al vèrtex p0
			float w1 = ((p2.y - p0.y) * (px - p2.x) + (p0.x - p2.x) * (py - p2.y)) / denom; //coordenada baricèntrica respecte al vèrtex p1 
			float w2 = 1.0f - w0 - w1; //coordenada baricèntrica respecte al vèrtex p2 (la suma de les coordenades baricèntriques ha de ser 1)

			//si alguna de les coordenades baricèntres es menor que -eps --> el píxel està fora del triangle (considerant una tolerància eps per evitar problemes numèrics) --> no dibuixem res
            if (w0 < -eps || w1 < -eps || w2 < -eps)
                continue;

			//clamp coordenades baricèntriques a 0 per evitar problemes numèrics en la interpolació del color 
			//coord baricentrica negativa = 0
            if (w0 < 0.0f) w0 = 0.0f; if (w1 < 0.0f) w1 = 0.0f; if (w2 < 0.0f) w2 = 0.0f;
            float sum = w0 + w1 + w2;
            if (sum <= 0.0f) continue;
            w0 /= sum; w1 /= sum; w2 /= sum;

			//interpolar color del píxel (x,y) com a combinació lineal dels colors dels vèrtexs del triangle, ponderada per les coordenades baricèntriques
            Color c = c0 * w0 + c1 * w1 + c2 * w2;

            SetPixel(x, y, c);
        }
    }
}
*/

void Image::DrawTriangleInterpolated(const sTriangleInfo& tri, FloatImage* zbuffer)
{
    const Vector3& p0 = tri.v[0];
    const Vector3& p1 = tri.v[1];
    const Vector3& p2 = tri.v[2];

    int minX = (int)floor(std::min({ p0.x, p1.x, p2.x }));
    int minY = (int)floor(std::min({ p0.y, p1.y, p2.y }));
    int maxX = (int)ceil(std::max({ p0.x, p1.x, p2.x }));
    int maxY = (int)ceil(std::max({ p0.y, p1.y, p2.y }));

    minX = std::max(0, minX);
    minY = std::max(0, minY);
    maxX = std::min((int)width - 1, maxX);
    maxY = std::min((int)height - 1, maxY);

    float denom = ((p1.y - p2.y) * (p0.x - p2.x) +
        (p2.x - p1.x) * (p0.y - p2.y));

    if (fabs(denom) < 1e-6f) return;

    const float eps = 1e-5f;

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            float px = x + 0.5f;
            float py = y + 0.5f;

            float w0 = ((p1.y - p2.y) * (px - p2.x) +
                (p2.x - p1.x) * (py - p2.y)) / denom;

            float w1 = ((p2.y - p0.y) * (px - p2.x) +
                (p0.x - p2.x) * (py - p2.y)) / denom;

            float w2 = 1.0f - w0 - w1;

            if (w0 < -eps || w1 < -eps || w2 < -eps)
                continue;

            if (w0 < 0) w0 = 0;
            if (w1 < 0) w1 = 0;
            if (w2 < 0) w2 = 0;

            float sum = w0 + w1 + w2;
            if (sum <= 0) continue;

            w0 /= sum; w1 /= sum; w2 /= sum;

            // Z-buffer
            float z = p0.z * w0 + p1.z * w1 + p2.z * w2;

			if (zbuffer) { //si tenim z-buffer --> comprovar si el píxel està ocult o no 
				//comparant la profunditat interpolada del píxel amb el valor actual del z-buffer en aquesta posicio
                float& zbuf = zbuffer->GetPixelRef(x, y);
                if (z > zbuf) continue; // pixel hidden
                zbuf = z;
            }

            // Texture or vertex color
            Color finalColor;
            if (tri.texture) {
                Vector2 uv = tri.uv[0] * w0 + tri.uv[1] * w1 + tri.uv[2] * w2;
                int tx = uv.x * (tri.texture->width - 1);
                int ty = uv.y * (tri.texture->height - 1);
                tx = std::max(0, std::min((int)tri.texture->width - 1, tx));
                ty = std::max(0, std::min((int)tri.texture->height - 1, ty));
                finalColor = tri.texture->GetPixel(tx, ty);
            }
            else {
                finalColor = tri.c[0] * w0 + tri.c[1] * w1 + tri.c[2] * w2;
            }

            SetPixel(x, y, finalColor);
        }
    }
}


Image::Image(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
	pixels = new Color[width*height];
	memset(pixels, 0, width * height * sizeof(Color));
}

// Copy constructor
Image::Image(const Image& c)
{
	pixels = NULL;
	width = c.width;
	height = c.height;
	bytes_per_pixel = c.bytes_per_pixel;
	if(c.pixels)
	{
		pixels = new Color[width*height];
		memcpy(pixels, c.pixels, width*height*bytes_per_pixel);
	}
}

// Assign operator
Image& Image::operator = (const Image& c)
{
	if(pixels) delete[] pixels;
	pixels = NULL;

	width = c.width;
	height = c.height;
	bytes_per_pixel = c.bytes_per_pixel;

	if(c.pixels)
	{
		pixels = new Color[width*height*bytes_per_pixel];
		memcpy(pixels, c.pixels, width*height*bytes_per_pixel);
	}
	return *this;
}

Image::~Image()
{
	if(pixels) 
		delete[] pixels;
}

void Image::Render()
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glDrawPixels(width, height, bytes_per_pixel == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

// Change image size (the old one will remain in the top-left corner)
void Image::Resize(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];
	unsigned int min_width = this->width > width ? width : this->width;
	unsigned int min_height = this->height > height ? height : this->height;

	for(unsigned int x = 0; x < min_width; ++x)
		for(unsigned int y = 0; y < min_height; ++y)
			new_pixels[ y * width + x ] = GetPixel(x,y);

	delete[] pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

// Change image size and scale the content
void Image::Scale(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];

	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++y)
			new_pixels[ y * width + x ] = GetPixel((unsigned int)(this->width * (x / (float)width)), (unsigned int)(this->height * (y / (float)height)) );

	delete[] pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

Image Image::GetArea(unsigned int start_x, unsigned int start_y, unsigned int width, unsigned int height)
{
	Image result(width, height);
	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++y)
		{
			if( (x + start_x) < this->width && (y + start_y) < this->height) 
				result.SetPixelUnsafe( x, y, GetPixel(x + start_x,y + start_y) );
		}
	return result;
}

void Image::FlipY()
{
	int row_size = bytes_per_pixel * width;
	Uint8* temp_row = new Uint8[row_size];
#pragma omp simd
	for (int y = 0; y < height * 0.5; y += 1)
	{
		Uint8* pos = (Uint8*)pixels + y * row_size;
		memcpy(temp_row, pos, row_size);
		Uint8* pos2 = (Uint8*)pixels + (height - y - 1) * row_size;
		memcpy(pos, pos2, row_size);
		memcpy(pos2, temp_row, row_size);
	}
	delete[] temp_row;
}

bool Image::LoadPNG(const char* filename, bool flip_y)
{
	std::string sfullPath = absResPath(filename);
	std::ifstream file(sfullPath, std::ios::in | std::ios::binary | std::ios::ate);

	// Get filesize
	std::streamsize size = 0;
	if (file.seekg(0, std::ios::end).good()) size = file.tellg();
	if (file.seekg(0, std::ios::beg).good()) size -= file.tellg();

	if (!size){
		std::cerr << "--- Failed to load file: " << sfullPath.c_str() << std::endl;
		return false;
	}

	std::vector<unsigned char> buffer;

	// Read contents of the file into the vector
	if (size > 0)
	{
		buffer.resize((size_t)size);
		file.read((char*)(&buffer[0]), size);
	}
	else
		buffer.clear();

	std::vector<unsigned char> out_image;

	if (decodePNG(out_image, width, height, buffer.empty() ? 0 : &buffer[0], (unsigned long)buffer.size(), true) != 0){
		std::cerr << "--- Failed to load file: " << sfullPath.c_str() << std::endl;
		return false;
	}

	size_t bufferSize = out_image.size();
	unsigned int originalBytesPerPixel = (unsigned int)bufferSize / (width * height);
	
	// Force 3 channels
	bytes_per_pixel = 3;

	if (originalBytesPerPixel == 3) {
		if (pixels) delete[] pixels;
		pixels = new Color[bufferSize];
		memcpy(pixels, &out_image[0], bufferSize);
	}
	else if (originalBytesPerPixel == 4) {
		if (pixels) delete[] pixels;

		unsigned int newBufferSize = width * height * bytes_per_pixel;
		pixels = new Color[newBufferSize];

		unsigned int k = 0;
		for (unsigned int i = 0; i < bufferSize; i += originalBytesPerPixel) {
			pixels[k] = Color(out_image[i], out_image[i + 1], out_image[i + 2]);
			k++;
		}
	}

	// Flip pixels in Y
	if (flip_y)
		FlipY();

	std::cout << "+++ File loaded: " << sfullPath.c_str() << std::endl;

	return true;
}

// Loads an image from a TGA file
bool Image::LoadTGA(const char* filename, bool flip_y)
{
	unsigned char TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char TGAcompare[12];
	unsigned char header[6];
	unsigned int imageSize;
	unsigned int bytesPerPixel;

    std::string sfullPath = absResPath( filename );

	FILE * file = fopen( sfullPath.c_str(), "rb");
   	if ( file == NULL || fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||
		fread(header, 1, sizeof(header), file) != sizeof(header))
	{
		std::cerr << "--- File not found: " << sfullPath.c_str() << std::endl;
		if (file == NULL)
			return NULL;
		else
		{
			fclose(file);
			return NULL;
		}
	}

	TGAInfo* tgainfo = new TGAInfo;
    
	tgainfo->width = header[1] * 256 + header[0];
	tgainfo->height = header[3] * 256 + header[2];
    
	if (tgainfo->width <= 0 || tgainfo->height <= 0 || (header[4] != 24 && header[4] != 32))
	{
		std::cerr << "--- Failed to load file: " << sfullPath.c_str() << std::endl;
		fclose(file);
		delete tgainfo;
		return NULL;
	}
    
	tgainfo->bpp = header[4];
	bytesPerPixel = tgainfo->bpp / 8;
	imageSize = tgainfo->width * tgainfo->height * bytesPerPixel;
    
	tgainfo->data = new unsigned char[imageSize];
    
	if (tgainfo->data == NULL || fread(tgainfo->data, 1, imageSize, file) != imageSize)
	{
		std::cerr << "--- Failed to load file: " << sfullPath.c_str() << std::endl;

		if (tgainfo->data != NULL)
			delete[] tgainfo->data;
            
		fclose(file);
		delete tgainfo;
		return false;
	}

	fclose(file);

	// Save info in image
	if(pixels)
		delete[] pixels;

	width = tgainfo->width;
	height = tgainfo->height;
	pixels = new Color[width*height];

	// Convert to float all pixels
	for (unsigned int y = 0; y < height; ++y) {
		for (unsigned int x = 0; x < width; ++x) {
			unsigned int pos = y * width * bytesPerPixel + x * bytesPerPixel;
			// Make sure we don't access out of memory
			if( (pos < imageSize) && (pos + 1 < imageSize) && (pos + 2 < imageSize))
				SetPixelUnsafe(x, height - y - 1, Color(tgainfo->data[pos + 2], tgainfo->data[pos + 1], tgainfo->data[pos]));
		}
	}

	// Flip pixels in Y
	if (flip_y)
		FlipY();

	delete[] tgainfo->data;
	delete tgainfo;

	std::cout << "+++ File loaded: " << sfullPath.c_str() << std::endl;

	return true;
}

// Saves the image to a TGA file
bool Image::SaveTGA(const char* filename)
{
	unsigned char TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	std::string fullPath = absResPath(filename);
	FILE *file = fopen(fullPath.c_str(), "wb");
	if ( file == NULL )
	{
		std::cerr << "--- Failed to save file: " << fullPath.c_str() << std::endl;
		return false;
	}

	unsigned short header_short[3];
	header_short[0] = width;
	header_short[1] = height;
	unsigned char* header = (unsigned char*)header_short;
	header[4] = 24;
	header[5] = 0;

	fwrite(TGAheader, 1, sizeof(TGAheader), file);
	fwrite(header, 1, 6, file);

	// Convert pixels to unsigned char
	unsigned char* bytes = new unsigned char[width*height*3];
	for(unsigned int y = 0; y < height; ++y)
		for(unsigned int x = 0; x < width; ++x)
		{
			Color c = pixels[y*width+x];
			unsigned int pos = (y*width+x)*3;
			bytes[pos+2] = c.r;
			bytes[pos+1] = c.g;
			bytes[pos] = c.b;
		}

	fwrite(bytes, 1, width*height*3, file);
	fclose(file);

	delete[] bytes;

	std::cout << "+++ File saved: " << fullPath.c_str() << std::endl;

	return true;
}

#ifndef IGNORE_LAMBDAS

// You can apply and algorithm for two images and store the result in the first one
// ForEachPixel( img, img2, [](Color a, Color b) { return a + b; } );
template <typename F>
void ForEachPixel(Image& img, const Image& img2, F f) {
	for(unsigned int pos = 0; pos < img.width * img.height; ++pos)
		img.pixels[pos] = f( img.pixels[pos], img2.pixels[pos] );
}

#endif

FloatImage::FloatImage(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
	pixels = new float[width * height];
	memset(pixels, 0, width * height * sizeof(float));
}

// Copy constructor
FloatImage::FloatImage(const FloatImage& c) {
	pixels = NULL;

	width = c.width;
	height = c.height;
	if (c.pixels)
	{
		pixels = new float[width * height];
		memcpy(pixels, c.pixels, width * height * sizeof(float));
	}
}

// Assign operator
FloatImage& FloatImage::operator = (const FloatImage& c)
{
	if (pixels) delete[] pixels;
	pixels = NULL;

	width = c.width;
	height = c.height;
	if (c.pixels)
	{
		pixels = new float[width * height * sizeof(float)];
		memcpy(pixels, c.pixels, width * height * sizeof(float));
	}
	return *this;
}

FloatImage::~FloatImage()
{
	if (pixels)
		delete[] pixels;
}

// Change image size (the old one will remain in the top-left corner)
void FloatImage::Resize(unsigned int width, unsigned int height)
{
	float* new_pixels = new float[width * height];
	unsigned int min_width = this->width > width ? width : this->width;
	unsigned int min_height = this->height > height ? height : this->height;

	for (unsigned int x = 0; x < min_width; ++x)
		for (unsigned int y = 0; y < min_height; ++y)
			new_pixels[y * width + x] = GetPixel(x, y);

	delete[] pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

//LAB 1 - PRIMITIVES
void Image::DrawLineDDA(int x0, int y0, int x1, int y1, const Color& c){ //2.1.1
	//implementar aqui el algoritme DDA per a dibuixar una linia
	int dx = x1 - x0; //diferencia de x
	int dy = y1 - y0; //diferencia de y
	int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy); //nombre de passos a fer
	if (steps == 0) { //si no hi ha passos, dibuixem un pixel
		SetPixel(x0, y0, c); //dibuixem un pixel
		return;
	}
	float x_increment = dx / (float)steps; //increment de x per cada pas
	float y_increment = dy / (float)steps; //increment de y per cada pas

	float x = (float)x0; //posicio inicial de x
	float y = (float)y0; //posicio inicial de y
	for (int i = 0; i <= steps; i++) {
		SetPixel((int)round(x), (int)round(y), c); //dibuixem el pixel a la posicio actual
		x += x_increment; //incrementem x
		y += y_increment; //incrementem y
	}
}

void Image::DrawRect(int x, int y, int w, int h, const Color& borderColor, int borderWidth, bool isFilled, const Color& fillColor) { //2.1.2
	//implementar aqui el algoritme per a dibuixar un rectangle
	if (isFilled) { //si el rectangle es ple
		for (int i = y; i < y + h; i++){ //per cada fila
			for (int j = x; j < x + w; j++){ //per cada columna
				SetPixel(j, i, fillColor); //dibuixem el pixel
			}
		}
	}
	//dibuixem el contorn --> isFilled = false
	for (int i = 0; i < borderWidth; i++){ //per cada pixel del contorn
		//superior
		DrawLineDDA(x, y + i, x + w - 1, y + i, borderColor);
		//inferior
		DrawLineDDA(x, y + h - 1 - i, x + w - 1, y + h - 1 - i, borderColor);
		//esquerra
		DrawLineDDA(x + i, y, x + i, y + h - 1, borderColor);
		//dreta 
		DrawLineDDA(x + w - 1 - i, y, x + w - 1 - i, y + h - 1, borderColor);
	}
}

//funció ScanLine per calcular els límits del triangle (AET)
void Image::ScanLineDDA(int x0, int y0, int x1, int y1, std::vector<int>& minX, std::vector<int>& maxX) { 
	int dx = x1 - x0; //diferència de x
	int dy = y1 - y0; //diferència de y
	//calculem quina distància és més gran (horitzontal o vertical) per saber quants passos píxels hem de recórrer
	int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

	//si els dos punts són el mateix, no fem res per evitar dividir per 0
	if (steps == 0) return;

	//calculem quant hem de sumar a la X i a la Y a cada pas per anar del punt A al B
	float x_increment = dx / (float)steps; //increment de x cada pas
	float y_increment = dy / (float)steps; //increment de y cada pas

	//coordds inicials (float per tenir precisió decimal mentre avancem).
	float x = (float)x0;
	float y = (float)y0;

	//recorrer la línia des del punt inicial fins al final
	for (int i = 0; i <= steps; i++){
		int currentY = (int)round(y); //pasar de float a int
		int currentX = (int)round(x); //pasar de float a int

		//només processem si estem dins de l'alçada de la imatge!!
		if (currentY >= 0 && currentY < height) { //comprovem que la Y està dins de la imatge
			if (currentX < minX[currentY]){ // minX[currentY] guarda el valor de X més a l'esquerra trobat fins ara a l'alçada Y
				minX[currentY] = currentX; //si la X actual és MÉS PETITA que la que teníem guardada, la substituïm
			} 
			
			if (currentX > maxX[currentY]){ //maxX[currentY] guarda el valor de X més a la dreta trobat fins ara
				maxX[currentY] = currentX; //si la X actual és MÉS GRAN, la substituïm
			}
		}
		//avancem al següent punt de la línia
		x += x_increment;
		y += y_increment;
	}
}

//funcio dibuixar triangles
void Image::DrawTriangle(const Vector2& p0, const Vector2& p1, const Vector2& p2, const Color& borderColor, bool isFilled, const Color& fillColor){
	//INTERIOR TRIANGLE
	if (isFilled) { 
		std::vector<int> minX(height, width); // minX l'omplim amb un valor molt alt perquè volem trobar el MÍNIM
		std::vector<int> maxX(height, -1); // maxX l'omplim amb un valor molt baix perquè volem trobar el MÀXIM
		//scanline per cada costat del triangle
		ScanLineDDA((int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, minX, maxX); //costat 1
		ScanLineDDA((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, minX, maxX); //costat 2
		ScanLineDDA((int)p2.x, (int)p2.y, (int)p0.x, (int)p0.y, minX, maxX); //costat 3

		for (int y = 0; y < height; y++){ //recorrem totes les files de la imatge
			if (minX[y] <= maxX[y] && maxX[y] != -1) { //si hi ha algun píxel a pintar en aquesta fila
				for (int x = minX[y]; x <= maxX[y]; x++) { //bucle horitzontal --> per cada píxel entre minX i maxX
					if (x >= 0 && x < width) { //mirem que la X està dins de la imatge
						SetPixel(x, y, fillColor); //pinta el píxel
					}
				}
			}
		}
	}

	//CONTORN TRIANGLE  
	DrawLineDDA((int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, borderColor); //costat 1
	DrawLineDDA((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, borderColor); //costat 2
	DrawLineDDA((int)p2.x, (int)p2.y, (int)p0.x, (int)p0.y, borderColor); //costat 3
}

//DRAWING TOOLS (2.2)
void Image::DrawImage(const Image& image, int x, int y){
	for (unsigned int i = 0; i < image.height; ++i) { //per cada fila de la imatge
		int dst_y = y + i; //posicio y de la imatge
		if (dst_y < 0 || dst_y >= (int)height) continue; //si esta fora de la imatge, saltem
		for (unsigned int j = 0; j < image.width; ++j) { //per cada columna de la imatge
			int dst_x = x + j; //posicio x de la imatge
			if (dst_x < 0 || dst_x >= (int)width) continue; //si esta fora de la imatge, saltem
			Color c = image.GetPixel(j, i); //obtenim el color del pixel de la imatge
			SetPixel(dst_x, dst_y, c); //posem el pixel a la posicio corresponent
		}
	}
}


