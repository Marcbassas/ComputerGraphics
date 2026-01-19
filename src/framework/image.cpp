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
void Image::DrawLineDDA(int x0, int y0, int x1, int y1, const Color& c) { //2.1.1
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
		for (int i = y; i < y + h; i++) { //per cada fila
			for (int j = x; j < x + w; j++) { //per cada columna
				SetPixel(j, i, fillColor); //dibuixem el pixel
			}
		}
	}
	//dibuixem el contorn --> isFilled = false
	for (int i = 0; i < borderWidth; i++) { //per cada pixel del contorn
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

// Funció ScanLine per calcular els límits del triangle (AET)
void Image::ScanLineDDA(int x0, int y0, int x1, int y1, std::vector<int>& minX, std::vector<int>& maxX) {
	// PREPARACIÓ DEL DDA (Digital Differential Analyzer) 
	int dx = x1 - x0;
	int dy = y1 - y0;
	// Calculem quina distància és més gran (horitzontal o vertical) per saber quants passos píxels hem de recórrer.
	int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

	// Si els dos punts són el mateix, no fem res per evitar dividir per 0.
	if (steps == 0) return;

	// Calculem quant hem de sumar a la X i a la Y a cada pas per anar del punt A al B.
	// (Per exemple: avançar 1 en X i 0.5 en Y).
	float x_increment = dx / (float)steps;
	float y_increment = dy / (float)steps;

	// Coordenades inicials (float per tenir precisió decimal mentre avancem).
	float x = (float)x0;
	float y = (float)y0;

	// EL BUCLE (Recorrem la línia) 
	for (int i = 0; i <= steps; i++) {
		// Convertim els decimals a enters (píxels reals de la pantalla).
		int currentY = (int)round(y);
		int currentX = (int)round(x);

		// ACTUALITZACIÓ DE LA TAULA 
		//(Només processem si estem dins de l'alçada de la imatge)!!
		if (currentY >= 0 && currentY < height) {

			if (currentX < minX[currentY]) { // minX[currentY] guarda el valor de X més a l'esquerra trobat fins ara a l'alçada Y.
				minX[currentY] = currentX; // Si la X actual és MÉS PETITA que la que teníem guardada, la substituïm.
			} // Així trobem on comensarà a pintar-se el triangle en aquesta línia.
			
			if (currentX > maxX[currentY]) { // maxX[currentY] guarda el valor de X més a la dreta trobat fins ara.
				maxX[currentY] = currentX; // Si la X actual és MÉS GRAN, la substituïm.
			} // Així trobem on acabarà el triangle en aquesta línia.
		}

		// Avancem al següent punt de la línia
		x += x_increment;
		y += y_increment;
	}
}

//Funció per dibuixar el triangle
void Image::DrawTriangle(const Vector2& p0, const Vector2& p1, const Vector2& p2, const Color& borderColor, bool isFilled, const Color& fillColor) {

	// 1. L'INTERIOR 
	if (isFilled) {
		// Creem dos vectors (arrays) amb tantes posicions com l'alçada de la pantalla.

		std::vector<int> minX(height, width); // minX l'omplim amb 'width' perquè volem trobar el valor MÍNIM.
		// Si posem un valor molt alt d'inici, qualsevol punt que trobem serà més petit i el guardarà.

		std::vector<int> maxX(height, -1); // maxX l'omplim amb -1 perquè volem trobar el MÀXIM
		//Començant amb un valor molt baix, qualsevol punt que trobem serà més gran i el guardarà.

		// Enviem el "ScanLineDDA" a recórrer les 3 parets del triangle.
		// Això omplirà les taules minX i maxX amb la forma del triangle.
		ScanLineDDA((int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, minX, maxX); // Costat 1
		ScanLineDDA((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, minX, maxX); // Costat 2
		ScanLineDDA((int)p2.x, (int)p2.y, (int)p0.x, (int)p0.y, minX, maxX); // Costat 3

		// Ara recorrem tota la pantalla de dalt a baix (fila per fila).
		for (int y = 0; y < height; y++) {

			// Comprovem si en aquesta fila 'y' hi ha triangle.
			// Si minX <= maxX, vol dir que hem trobat un inici i un final vàlids.
			if (minX[y] <= maxX[y] && maxX[y] != -1) {

				// Bucle horitzontal: Pintem tots els píxels des del límit esquerre (min) fins al dret (max).
				for (int x = minX[y]; x <= maxX[y]; x++) {

					// (Mirem que no pinti fora de la pantalla lateralment)!
					if (x >= 0 && x < width) {
						SetPixel(x, y, fillColor); // Pinta el píxel de color.
					}
				}
			}
		}
	}

	// 2. EL CONTORN 
	// Finalment, dibuixem les línies exteriors a sobre. 
	// Per això fem servir la funció simple DrawLineDDA.
	DrawLineDDA((int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, borderColor);
	DrawLineDDA((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, borderColor);
	DrawLineDDA((int)p2.x, (int)p2.y, (int)p0.x, (int)p0.y, borderColor);
}
