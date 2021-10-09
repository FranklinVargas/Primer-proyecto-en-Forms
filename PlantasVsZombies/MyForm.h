#pragma once

#include "Juego.hpp"

namespace PlantasVsZombies {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class MyForm : public System::Windows::Forms::Form
	{
		Graphics^ graficador;
		BufferedGraphics^ buffer;
	private: System::Windows::Forms::Timer^ tim_disparador;
		   CJuego^ juego;
	public:
		MyForm(void)
		{
			InitializeComponent();
			this->graficador = this->CreateGraphics();
			this->buffer = BufferedGraphicsManager::Current->Allocate(this->graficador, 
				                                                      this->ClientRectangle);
			this->juego = gcnew CJuego(this->ClientRectangle);
		}
	protected:
		~MyForm()
		{
			delete this->juego;
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Timer^ tim_animador;
	private: System::ComponentModel::IContainer^ components;
	private:
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->tim_animador = (gcnew System::Windows::Forms::Timer(this->components));
			this->tim_disparador = (gcnew System::Windows::Forms::Timer(this->components));
			this->SuspendLayout();
			// 
			// tim_animador
			// 
			this->tim_animador->Enabled = true;
			this->tim_animador->Interval = 50;
			this->tim_animador->Tick += gcnew System::EventHandler(this, &MyForm::animar);
			// 
			// tim_disparador
			// 
			this->tim_disparador->Enabled = true;
			this->tim_disparador->Interval = 2000;
			this->tim_disparador->Tick += gcnew System::EventHandler(this, &MyForm::disparar);
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(11, 24);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1275, 598);
			this->Name = L"MyForm";
			this->Text = L"MyForm";
			this->WindowState = System::Windows::Forms::FormWindowState::Maximized;
			this->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::drag);
			this->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::move);
			this->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::drop);
			this->ResumeLayout(false);

		}
		private: System::Void animar(System::Object^ sender, System::EventArgs^ e) {
			this->juego->jugar(this->buffer->Graphics);
			this->buffer->Render();
		}
		private: System::Void disparar(System::Object^ sender, System::EventArgs^ e) {
			this->juego->agregar_bala();
			this->tim_disparador->Interval = 750;
		}
		private: System::Void drag(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
			this->juego->drag_planta_base(e->X, e->Y);
		}
		private: System::Void drop(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
			this->juego->drop_planta_base(); 
		}
		private: System::Void move(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
			this->juego->mover_planta_base(e->X, e->Y);
		}
	};
}
