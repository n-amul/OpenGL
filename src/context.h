#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "buffer.h"
#include "common.h"
#include "program.h"
#include "shader.h"
#include "texture.h"
#include "vertex_layout.h"
#include <time.h>

CLASS_PTR(Context)

class Context
{
  public:
    ~Context();
    static ContextUPtr Create();
    void Render();
    void ProcessInput(GLFWwindow *window);
    void Reshape(int width, int height);
    void MouseMove(double x, double y);
    void MouseButton(int button, int action, double x, double y);

  private:
    Context(){};
    bool Init();
    VertexLayoutUPtr m_vertexLayout;
    BufferUPtr m_indexBuffer;
    BufferUPtr m_vertexBuffer;

    ProgramUPtr m_program;
    TextureUPtr m_texture;
    TextureUPtr m_texture2;

    // camera parameter
    glm::vec3 m_cameraPos{glm::vec3(0.0f, 0.0f, 3.0f)};
    glm::vec3 m_cameraFront{glm::vec3(0.0f, 0.0f, -1.0f)};
    glm::vec3 m_cameraUp{glm::vec3(0.0f, 1.0f, 0.0f)};
    bool m_cameraControl{false};
    glm::vec2 m_prevMousePos{glm::vec2(0.0f)};
    float m_cameraPitch{0.0f};
    float m_cameraYaw{0.0f};

    int m_width{640};
    int m_height{480};
};
#endif