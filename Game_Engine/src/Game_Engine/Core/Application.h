#pragma once

#include "Core.h"

#include "Window.h"
#include "Game_Engine/Core/LayerStack.h"
#include "Game_Engine/Events/Event.h"
#include "Game_Engine/Events/ApplicationEvent.h"

#include "Game_Engine/Core/Timestep.h"

#include "Game_Engine/ImGui/ImGuiLayer.h"

namespace GameEngine {

	class Application {
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline Window& GetWindow() { return *m_Window; }
		inline static Application& Get() { return *s_Instance; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}
