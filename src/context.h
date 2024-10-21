#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "buffer.h"
#include "common.h"
#include "program.h"
#include "shader.h"
#include "texture.h"
#include "vertex_layout.h"
#include "mesh.h"
#include "model.h"
#include "framebuffer.h"
#include "shadow_map.h"
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

    ProgramUPtr m_program;
    ProgramUPtr m_simpleProgram;
    ProgramUPtr m_textureProgram;
    ProgramUPtr m_postProgram;
    ProgramUPtr m_lightingShadowProgram;
    float m_gamma {1.0f};

    MeshUPtr m_box;
    MeshUPtr m_plane;
   	ModelUPtr m_model;

    MeshUPtr m_smallBox;
    MaterialPtr m_smallBoxMaterial;

    void DrawScene(const glm::mat4& view, const glm::mat4& projection,const Program* program);

    //  animation
    bool m_animation{true};

    // camera parameter
    glm::vec3 m_cameraPos{glm::vec3(0.0f, 2.5f, 8.0f)};
    glm::vec3 m_cameraFront{glm::vec3(0.0f, 0.0f, -1.0f)};
    glm::vec3 m_cameraUp{glm::vec3(0.0f, 1.0f, 0.0f)};
    bool m_cameraControl{false};
    
    glm::vec2 m_prevMousePos{glm::vec2(0.0f)};
    float m_cameraPitch{-20.0f};
    float m_cameraYaw{0.0f};
    // framebuffer
    FramebufferUPtr m_framebuffer;
    FramebufferUPtr m_framebufferMSAA;
    //shadow map
    ShadowMapUPtr m_shadowMap;
    
    // clear color
    glm::vec4 m_clearColor{glm::vec4(0.1f, 0.2f, 0.3f, 0.0f)};
    // light parameter
    struct Light
    {
        bool directional { false };
        glm::vec3 position { glm::vec3(2.0f, 4.0f, 4.0f) };
        glm::vec3 direction { glm::vec3(-0.5f, -1.5f, -1.0f) };
        glm::vec2 cutoff { glm::vec2(50.0f, 5.0f) };
        float distance { 150.0f };
        glm::vec3 ambient { glm::vec3(0.1f, 0.1f, 0.1f) };
        glm::vec3 diffuse { glm::vec3(0.8f, 0.8f, 0.8f) };
        glm::vec3 specular { glm::vec3(1.0f, 1.0f, 1.0f) };
    };
    Light m_light;

    bool m_blinn{true};
    // material parameter
    MaterialPtr m_material;
    MaterialPtr m_planeMaterial;

    TexturePtr m_windowTexture;
    // cubemap
    CubeTextureUPtr m_cubeTexture;
    ProgramUPtr m_skyboxProgram;
    //grass instance
    TexturePtr m_grassTexture;
    ProgramUPtr m_grassProgram;
    std::vector<glm::vec3> m_grassPos;
    BufferUPtr m_grassPosBuffer;
    VertexLayoutUPtr m_grassInstance;
    

    int m_width{640};
    int m_height{480};
};
#endif