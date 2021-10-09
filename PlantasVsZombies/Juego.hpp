#pragma once

#include <Windows.h>
#include <MMSystem.h>

using namespace System; //String^
using namespace System::Drawing; //Bitmap^, Rectangle
using namespace System::Collections::Generic; //List^
using namespace System::Windows::Forms; //MessageBox::Show();

ref class CGrafico abstract {
protected:
	System::Drawing::Rectangle area_dibujo;
	short dx, dy;
public:
	CGrafico(System::Drawing::Rectangle area_dibujo) 
		: area_dibujo(area_dibujo), dx(0), dy(0) {}
	void set_dx(short dx) { this->dx = dx; }
	void set_dy(short dy) { this->dy = dy; }
	void renderizar(Graphics^ graficador) {
		this->dibujar(graficador);
		this->mover();
	}
	bool hay_colision(CGrafico^ otro) { 
		return this->area_dibujo.IntersectsWith(otro->area_dibujo);
	}
protected:
	virtual void dibujar(Graphics^ graficador) abstract;
	void mover() {
		this->area_dibujo.X += this->dx;
		this->area_dibujo.Y += this->dy;
	}
};
ref class CBala : public CGrafico {
public:
	CBala(System::Drawing::Rectangle area_dibujo) 
		: CGrafico(area_dibujo) {
		this->dx = 5;
	}
	bool salio(short limite_x) {
		return this->area_dibujo.X > limite_x;
	}
protected:
	void dibujar(Graphics^ graficador) override {
		SolidBrush^ brocha = gcnew SolidBrush(Color::Green);
		graficador->FillEllipse(brocha, this->area_dibujo);
	}
};

ref class CImagen : public CGrafico {
private:
	bool imagen_propia;
protected:
	Bitmap^ imagen;
public:
	CImagen(String^ ruta, System::Drawing::Rectangle area_dibujo)
		: CGrafico(area_dibujo) {
		this->imagen = gcnew Bitmap(ruta); //crea una imagen
		this->imagen_propia = true;
	}
	CImagen(Bitmap^ imagen, System::Drawing::Rectangle area_dibujo)
		: CGrafico(area_dibujo) {
		this->imagen = imagen;//apunta a una imagen creada
		this->imagen_propia = false;
	}
	~CImagen() {
		if(this->imagen_propia)
			delete this->imagen;
	}
	short get_ancho() { return this->imagen->Width; }
	void hacer_transparente() {
		Color color = this->imagen->GetPixel(0, 0);
		this->imagen->MakeTransparent(color);
	}
protected:
	virtual void dibujar(Graphics^ graficador) override {
		graficador->DrawImage(this->imagen, this->area_dibujo);
	}
};

//¡Abstracción! El nombre de la siguiente clase y su definición, 
//no son tan acordes, ¿qué deberíamos hacer? Un sprite, no involucra
//el concepto de vidas
ref class CSprite : public CImagen {
	short n_filas;
	short n_columnas;
	short n_subimagenes;
	short indice;
	short vidas;
public:
	CSprite(String^ ruta, System::Drawing::Rectangle area_dibujo, short n_f, short n_c, short n_subi, short vidas) 
		: CImagen(ruta, area_dibujo), n_filas(n_f), n_columnas(n_c), n_subimagenes(n_subi), vidas(vidas), indice(0) {
		this->hacer_transparente();
	}
	CSprite(Bitmap^ imagen, System::Drawing::Rectangle area_dibujo, short n_f, short n_c, short n_subi, short vidas)
		: CImagen(imagen, area_dibujo), n_filas(n_f), n_columnas(n_c), n_subimagenes(n_subi), vidas(vidas), indice(0) {
		this->hacer_transparente();
	}
	bool perder_vida() { --this->vidas; return this->vidas == 0; }
	void set_ubicacion(short x, short y) {
		this->area_dibujo.X = x;
		this->area_dibujo.Y = y;
	}
	bool esta_dentro(short x, short y) {
		return (x >= this->area_dibujo.X && x <= this->area_dibujo.X + this->area_dibujo.Width &&
			    y >= this->area_dibujo.Y && y <= this->area_dibujo.Y + this->area_dibujo.Height);
	}
protected:
	void dibujar(Graphics^ graficador) override {
		System::Drawing::Rectangle area_recorte = calcular_area_recorte();
		graficador->DrawImage(this->imagen, this->area_dibujo, area_recorte, GraphicsUnit::Pixel);
		++indice %= n_subimagenes;
	}
private:
	System::Drawing::Rectangle calcular_area_recorte() {
		short ancho_subimagen = this->imagen->Width / this->n_columnas;
		short alto_subimagen  = this->imagen->Height / this->n_filas;
		short x = this->indice % this->n_columnas * ancho_subimagen;
		short y = this->indice / this->n_columnas * alto_subimagen;
		return System::Drawing::Rectangle(x, y, ancho_subimagen, alto_subimagen);
	}
};

ref class CJuego {
	CImagen^ escenario;
	CSprite^ planta_base;
	CSprite^ planta;
	CSprite^ zombie;
	Bitmap^ imagen_planta;
	List<CBala^>^ balas;
	short dificultad;
	bool movimiento_planta;
public:
	CJuego(System::Drawing::Rectangle area_dibujo) {
		PlaySound(TEXT("Musica\\PlantasVsZombies.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
		this->escenario = gcnew CImagen("Imagenes\\escenario.jpg", area_dibujo);
		this->imagen_planta = gcnew Bitmap("Imagenes\\planta.jpg");
		this->planta_base = gcnew CSprite(this->imagen_planta, System::Drawing::Rectangle(0, 0, 100, 125), 4, 4, 15, 0);
		this->planta       = gcnew CSprite(this->imagen_planta, System::Drawing::Rectangle(260, 210, 100, 125), 4, 4, 15, 5);
		this->balas = gcnew List<CBala^>;
		this->zombie = nullptr;
		this->dificultad = 2;
		this->movimiento_planta = false;
	}
	~CJuego() {
		PlaySound(NULL, NULL, 0);
		for (short i = 0; i < this->balas->Count; ++i)
			delete this->balas[i];
		delete this->balas, this->escenario, this->planta, this->planta_base, this->imagen_planta, this->zombie;
	}
	bool jugar(Graphics^ graficador) {
		this->crear_zombie();
		this->renderizar(graficador);
		this->eliminar_graficos();
		return true;
	}
	void agregar_bala() {
		CBala^ bala = gcnew CBala(System::Drawing::Rectangle(350, 250, 35, 35));
		this->balas->Add(bala);
	}
	void drag_planta_base(short x, short y) { if(this->planta_base->esta_dentro(x, y)) this->movimiento_planta = true; }
	void drop_planta_base() { this->movimiento_planta = false; }
	void mover_planta_base(short x, short y) {
		if(this->movimiento_planta)
			this->planta_base->set_ubicacion(x, y);
	}
private:
	void crear_zombie() {
		if (this->zombie == nullptr) {
			this->zombie = gcnew CSprite("Imagenes\\zombie.jpg", System::Drawing::Rectangle(800, 210, 100, 125), 4, 6, 20, this->dificultad++);
			this->zombie->set_dx(this->dificultad*-1);
		}
	}
	void renderizar(Graphics^ graficador) {
		this->escenario->renderizar(graficador);
		this->planta_base->renderizar(graficador);
		this->planta->renderizar(graficador);
		this->zombie->renderizar(graficador);
		for(short i=0; i<this->balas->Count; ++i)
			this->balas[i]->renderizar(graficador);
	}
	void eliminar_graficos() {
		for (short i = this->balas->Count - 1; i >= 0; --i) {
			if (this->balas[i]->salio(this->escenario->get_ancho()) ||
				this->balas[i]->hay_colision(this->zombie)) {
				if (this->zombie && this->balas[i]->hay_colision(this->zombie)){
					if (this->zombie->perder_vida()) {
						delete this->zombie;
						this->zombie = nullptr;
					}
				}
				delete this->balas[i];
				this->balas->RemoveAt(i);
			}
		}
	}
};