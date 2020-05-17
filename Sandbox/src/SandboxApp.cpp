#include <GameEngine.h>

#include "Platform/OpenGL/OpenGLShader.h"

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ExampleLayer : public GameEngine::Layer {
public:
	ExampleLayer()
		: GameEngine::Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f) {
		m_VertexArray = GameEngine::VertexArray::Create();

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f,  0.808f, 0.416f, 0.522f, 1.0f,
			 0.5f, -0.5f, 0.0f,  0.965f, 0.847f, 0.682f, 1.0f,
			 0.0f,  0.5f, 0.0f,  0.102f, 0.784f, 0.929f, 1.0f,
		};

		GameEngine::Ref<GameEngine::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(GameEngine::VertexBuffer::Create(vertices, sizeof(vertices)));

		{
			GameEngine::BufferLayout layout = {
				{ GameEngine::ShaderDataType::Float3, "a_Position" },
				{ GameEngine::ShaderDataType::Float4, "a_Color" },
			};

			vertexBuffer->SetLayout(layout);
		}

		m_VertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		GameEngine::Ref<GameEngine::IndexBuffer> indexBuffer;
		indexBuffer.reset(GameEngine::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->AddIndexBuffer(indexBuffer);

		std::string vertexSrc = R"(
			#version 330 core
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			
			out vec3 v_Position;
			out vec4 v_Color;

			void main() {
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSrc = R"(
			#version 330 core
			layout(location = 0) out vec4 color;
			
			in vec3 v_Position;
			in vec4 v_Color;

			void main() {
				color = v_Color;
			}
		)";

		m_Shader = GameEngine::Shader::Create("VertexPosColor", vertexSrc, fragmentSrc);

		m_SquareVA = GameEngine::VertexArray::Create();

		float squareVertices[4 * 5] = {
			-0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
		};

		GameEngine::Ref<GameEngine::VertexBuffer> squareVB;
		squareVB.reset(GameEngine::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ GameEngine::ShaderDataType::Float3, "a_Position" },
			{ GameEngine::ShaderDataType::Float2, "a_TexCoord" },
		});

		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		GameEngine::Ref<GameEngine::IndexBuffer> squareIB;
		squareIB.reset(GameEngine::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->AddIndexBuffer(squareIB);

		std::string flatColorShaderVertexSrc = R"(
			#version 330 core
			layout(location = 0) in vec3 a_Position;
			
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;

			void main() {
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string flatColorShaderFragmentSrc = R"(
			#version 330 core
			layout(location = 0) out vec4 color;
			
			in vec3 v_Position;

			uniform vec3 u_Color;

			void main() {
				color = vec4(u_Color, 1.0f);
			}
		)";

		m_FlatColorShader = GameEngine::Shader::Create("FlatColor", flatColorShaderVertexSrc, flatColorShaderFragmentSrc);

		auto textureShader = m_ShaderLibrary.Load("assets/shaders/texture.glsl");

		m_Texture = GameEngine::Texture2D::Create("assets/textures/ink_cloud.png");

		std::dynamic_pointer_cast<GameEngine::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<GameEngine::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnUpdate(GameEngine::Timestep ts) override {

		if (GameEngine::Input::IsKeyPressed(GE_KEY_LEFT)) {
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;
		} else if (GameEngine::Input::IsKeyPressed(GE_KEY_RIGHT)) {
			m_CameraPosition.x += m_CameraMoveSpeed * ts;
		}

		if (GameEngine::Input::IsKeyPressed(GE_KEY_UP)) {
			m_CameraPosition.y += m_CameraMoveSpeed * ts;
		} else if (GameEngine::Input::IsKeyPressed(GE_KEY_DOWN)) {
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;
		}

		if (GameEngine::Input::IsKeyPressed(GE_KEY_A)) {
			m_CameraRotation += m_CameraRotationSpeed * ts;
		} else if (GameEngine::Input::IsKeyPressed(GE_KEY_D)) {
			m_CameraRotation -= m_CameraRotationSpeed * ts;
		}

		GameEngine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		GameEngine::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);
		
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		GameEngine::Renderer::BeginScene(m_Camera);

		glm::vec4 redColor(0.8f, 0.2f, 0.3f, 1.0f);
		glm::vec4 blueColor(0.2f, 0.2f, 0.8f, 1.0f);

		std::dynamic_pointer_cast<GameEngine::OpenGLShader>(m_FlatColorShader)->Bind();
		std::dynamic_pointer_cast<GameEngine::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", m_SquareColor);

		for (int y = 0; y < 20; y++) {
			for (int x = 0; x < 20; x++) {
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				GameEngine::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
			}
		}
		auto textureShader = m_ShaderLibrary.Get("texture");
		
		m_Texture->Bind();
		GameEngine::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		GameEngine::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override {
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}
	
	void OnEvent(GameEngine::Event& event) override {
	}

private:
	GameEngine::ShaderLibrary m_ShaderLibrary;
	GameEngine::Ref<GameEngine::Shader> m_Shader;
	GameEngine::Ref<GameEngine::VertexArray> m_VertexArray;

	GameEngine::Ref<GameEngine::Shader> m_FlatColorShader;
	GameEngine::Ref<GameEngine::VertexArray> m_SquareVA;

	GameEngine::Ref<GameEngine::Texture2D> m_Texture;

	GameEngine::OrthographicCamera m_Camera;

	glm::vec3 m_CameraPosition;
	float m_CameraMoveSpeed = 2.0f;

	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 180.0f;

	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};

class Sandbox : public GameEngine::Application {
	public:
		Sandbox() {
			PushLayer(new ExampleLayer());
		}
	
		~Sandbox() {}
};

GameEngine::Application* GameEngine::CreateApplication() {
	return new Sandbox();
}