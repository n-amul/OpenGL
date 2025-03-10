#include "context.h"
#include "image.h"
#include <imgui.h>

Context::~Context()
{
}

ContextUPtr Context::Create()
{
    auto context = ContextUPtr(new Context());
    if (!context->Init())
    {
        return nullptr;
    }
    return move(context);
}
void Context::Reshape(int width, int height)
{
    m_width = width;
    m_height = height;
    glViewport(0, 0, m_width, m_height);

    // Create MSAA framebuffer
    m_framebufferMSAA = Framebuffer::CreateMSAA(width, height, GL_RGBA);  // using 4x MSAA
    m_framebuffer = Framebuffer::Create(Texture::Create(width, height, GL_RGBA));
}
void Context::MouseMove(double x, double y)
{
    if (!m_cameraControl)
        return;
    auto pos = glm::vec2((float)x, (float)y);
    auto deltaPos = pos - m_prevMousePos;

    const float cameraRotSpeed = 0.8f;
    m_cameraYaw -= deltaPos.x * cameraRotSpeed;
    m_cameraPitch -= deltaPos.y * cameraRotSpeed;

    if (m_cameraYaw < 0.0f)
        m_cameraYaw += 360.0f;
    if (m_cameraYaw > 360.0f)
        m_cameraYaw -= 360.0f;

    if (m_cameraPitch > 89.0f)
        m_cameraPitch = 89.0f;
    if (m_cameraPitch < -89.0f)
        m_cameraPitch = -89.0f;

    m_prevMousePos = pos;
}
void Context::MouseButton(int button, int action, double x, double y)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            // 마우스 조작 시작 시점에 현재 마우스 커서 위치 저장
            m_prevMousePos = glm::vec2((float)x, (float)y);
            m_cameraControl = true;
        }
        else if (action == GLFW_RELEASE)
        {
            m_cameraControl = false;
        }
    }
}
bool Context::Init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
    m_shadowMap=ShadowMap::Create(1024,1024);
    m_box=Mesh::CreateBox();
    m_smallBox=Mesh::CreateBox();

    // Log the current working directory
    std::filesystem::path current_path = std::filesystem::current_path();
    SPDLOG_INFO("Current working directory: {}", current_path.string());

    // create program
    m_simpleProgram = Program::Create("../../shader/simple.vs", "../../shader/simple.fs");
    if (!m_simpleProgram)
        return false;

    m_program = Program::Create("../../shader/lighting.vs", "../../shader/lighting.fs");
    if (!m_program)
        return false;

    SPDLOG_INFO("program ID: {}", m_program->Get());
    m_model = Model::Load("../../model/backpack.obj");
    if (!m_model){
         return false;
    }
    m_plane = Mesh::CreatePlane();

     m_textureProgram = Program::Create("../../shader/texture.vs", "../../shader/texture.fs");
    if (!m_textureProgram)
      return false;

    m_postProgram = Program::Create("../../shader/texture.vs", "../../shader/gamma.fs");
    if (!m_postProgram)
        return false;
    
    m_grassProgram=Program::Create("../../shader/grass.vs", "../../shader/grass.fs");
    if(!m_grassProgram){
        return false;
    }
    m_lightingShadowProgram=Program::Create("../../shader/lighting_shadow.vs","../../shader/lighting_shadow.fs");
    if(!m_lightingShadowProgram){
        return false;
    }
    
    //load texture
    auto cubeRight = Image::Load("../../image/skybox/right.jpg", false);
    auto cubeLeft = Image::Load("../../image/skybox/left.jpg", false);
    auto cubeTop = Image::Load("../../image/skybox/top.jpg", false);
    auto cubeBottom = Image::Load("../../image/skybox/bottom.jpg", false);
    auto cubeFront = Image::Load("../../image/skybox/front.jpg", false);
    auto cubeBack = Image::Load("../../image/skybox/back.jpg", false);
    m_cubeTexture = CubeTexture::CreateFromImages({
        cubeRight.get(),
        cubeLeft.get(),
        cubeTop.get(),
        cubeBottom.get(),
        cubeFront.get(),
        cubeBack.get(),
    });
    m_skyboxProgram = Program::Create("../../shader/skybox.vs", "../../shader/skybox.fs");
    m_windowTexture = Texture::CreateFromImage(Image::Load("../../image/blending_transparent_window.png").get());
    m_grassTexture=Texture::CreateFromImage(Image::Load("../../image/grass.png").get());

    TexturePtr grayTexture = Texture::CreateFromImage(Image::CreateSingleColorImage(4, 4, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)).get());
    m_material = Material::Create();
    m_material->diffuse = Texture::CreateFromImage( Image::CreateSingleColorImage(4, 4, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)).get()); //gen & bind
    m_material->specular = grayTexture;


    m_planeMaterial = Material::Create();
    m_planeMaterial->diffuse = Texture::CreateFromImage(Image::Load("../../image/marble.jpg").get());
    m_planeMaterial->specular = grayTexture;
    m_planeMaterial->shininess = 128.0f;

    m_smallBoxMaterial = Material::Create();
    m_smallBoxMaterial->diffuse = Texture::CreateFromImage(Image::Load("../../image/container2.png").get());
    m_smallBoxMaterial->specular = Texture::CreateFromImage(Image::Load("../../image/container2_specular.png").get());
    m_smallBoxMaterial->shininess = 60.0f;

    m_grassPos.resize(10000);
    for (size_t i = 0; i < m_grassPos.size(); i++) {
        m_grassPos[i].x = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * 5.0f;
        m_grassPos[i].z = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * 5.0f;
        m_grassPos[i].y = glm::radians((float)rand() / (float)RAND_MAX * 360.0f);
    }
    m_grassInstance = VertexLayout::Create();
    m_grassInstance->Bind();
    m_plane->GetVertexBuffer()->Bind();
    m_grassInstance->SetAttrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    m_grassInstance->SetAttrib(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, normal));
    m_grassInstance->SetAttrib(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, texCoord));
    
    m_grassPosBuffer = Buffer::CreateWithData(GL_ARRAY_BUFFER, GL_STATIC_DRAW, m_grassPos.data(), sizeof(glm::vec3), m_grassPos.size());
    m_grassPosBuffer->Bind();
    m_grassInstance->SetAttrib(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
    glVertexAttribDivisor(3, 1);
    m_plane->GetIndexBuffer()->Bind();

    return true;
}
void Context::Render()
{
    if (ImGui::Begin("ui window")) {
        if (ImGui::CollapsingHeader("light", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("l.position", glm::value_ptr(m_light.position), 0.01f);
            ImGui::DragFloat3("l.direction", glm::value_ptr(m_light.direction), 0.01f);
            ImGui::DragFloat2("l.cutoff", glm::value_ptr(m_light.cutoff), 0.1f, 0.0f, 180.0f);
            ImGui::DragFloat("l.distance", &m_light.distance, 0.1f, 0.0f, 1000.0f);
            ImGui::ColorEdit3("l.ambient", glm::value_ptr(m_light.ambient));
            ImGui::ColorEdit3("l.diffuse", glm::value_ptr(m_light.diffuse));
            ImGui::ColorEdit3("l.specular", glm::value_ptr(m_light.specular));
            ImGui::Checkbox("l.blinn", &m_blinn);
            ImGui::Checkbox("l.directional light", &m_light.directional);
        }

        if (ImGui::CollapsingHeader("material", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat("m.shininess", &m_material->shininess, 1.0f, 1.0f, 256.0f);
        }

        ImGui::Checkbox("animation", &m_animation);

        if (ImGui::ColorEdit4("clear color", glm::value_ptr(m_clearColor))) {
            glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
        }
        ImGui::DragFloat("gamma", &m_gamma, 0.01f, 0.0f, 2.0f);

        ImGui::Separator();
        ImGui::DragFloat3("camera pos", glm::value_ptr(m_cameraPos), 0.01f);
        ImGui::DragFloat("camera yaw", &m_cameraYaw, 0.5f);
        ImGui::DragFloat("camera pitch", &m_cameraPitch, 0.5f, -89.0f, 89.0f);
        ImGui::Separator();
        if (ImGui::Button("reset camera")) {
            m_cameraYaw = 0.0f;
            m_cameraPitch = 0.0f;
            m_cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
        }
        ImGui::Image((ImTextureID)m_shadowMap->GetShadowMap()->Get(),ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();

    //shadow mapping
    auto lightView = glm::lookAt(m_light.position, m_light.position + m_light.direction,glm::vec3(0.0f, 1.0f, 0.0f));
    auto lightProjection = m_light.directional ?
        glm::ortho(-10.0f,10.0f,-10.0f,10.0f , 1.0f, 30.0f):
        glm::perspective(glm::radians((m_light.cutoff[0] + m_light.cutoff[1]) * 2.0f), 1.0f, 1.0f, 20.0f);
    m_shadowMap->Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0,m_shadowMap->GetShadowMap()->GetWidth(),m_shadowMap->GetShadowMap()->GetHeight());
    m_simpleProgram->Use();
    m_simpleProgram->SetUniform("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    DrawScene(lightView, lightProjection, m_simpleProgram.get());

    Framebuffer::BindToDefault();
    glViewport(0, 0, m_width, m_height);

    //draw on MSAA frame buffer
    m_framebufferMSAA->Bind(); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_cameraFront = glm::rotate(glm::mat4(1.0f), glm::radians(m_cameraYaw), glm::vec3(0.0f, 1.0f, 0.0f)) *
                    glm::rotate(glm::mat4(1.0f), glm::radians(m_cameraPitch), glm::vec3(1.0f, 0.0f, 0.0f)) *
                    glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

    auto view = glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
    auto projection = glm::perspective(glm::radians(45.0f), (float)(m_width / m_height), 0.01f, 100.0f);
    
    //skybox
    auto skyboxModelTransform =glm::translate(glm::mat4(1.0), m_cameraPos) * glm::scale(glm::mat4(1.0), glm::vec3(50.0f));
    m_skyboxProgram->Use();
    m_cubeTexture->Bind();
    m_skyboxProgram->SetUniform("skybox", 0);
    m_skyboxProgram->SetUniform("transform", projection * view * skyboxModelTransform);
    m_box->Draw(m_skyboxProgram.get());
    
    //  light cube
    auto lightModelTransform =
        glm::translate(glm::mat4(1.0), m_light.position) *
        glm::scale(glm::mat4(1.0), glm::vec3(0.1f));
    m_simpleProgram->Use();
    m_simpleProgram->SetUniform("color", glm::vec4(m_light.ambient + m_light.diffuse, 1.0f));
    m_simpleProgram->SetUniform("transform", projection * view * lightModelTransform);
    m_box->Draw(m_simpleProgram.get());
    
    //setup lighting shader var
    m_lightingShadowProgram->Use();
    m_lightingShadowProgram->SetUniform("viewPos", m_cameraPos);
    m_lightingShadowProgram->SetUniform("light.position", m_light.position);
    m_lightingShadowProgram->SetUniform("light.direction", m_light.direction);
    m_lightingShadowProgram->SetUniform("light.cutoff", glm::vec2(
        cosf(glm::radians(m_light.cutoff[0])),
        cosf(glm::radians(m_light.cutoff[0] + m_light.cutoff[1]))));
    m_lightingShadowProgram->SetUniform("light.attenuation", GetAttenuationCoeff(m_light.distance));
    m_lightingShadowProgram->SetUniform("light.ambient", m_light.ambient);
    m_lightingShadowProgram->SetUniform("light.diffuse", m_light.diffuse);
    m_lightingShadowProgram->SetUniform("light.specular", m_light.specular);
    m_lightingShadowProgram->SetUniform("blinn", (m_blinn ? 1 : 0));
    m_lightingShadowProgram->SetUniform("lightTransform", lightProjection * lightView);
    m_lightingShadowProgram->SetUniform("light.directional", m_light.directional ? 1 : 0);//Todo: oversampling
    glActiveTexture(GL_TEXTURE3);
    m_shadowMap->GetShadowMap()->Bind();
    m_lightingShadowProgram->SetUniform("shadowMap", 3);
    glActiveTexture(GL_TEXTURE0);

    DrawScene(view, projection, m_lightingShadowProgram.get());


    // window with blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_textureProgram->Use();
    m_windowTexture->Bind();
    m_textureProgram->SetUniform("tex", 0);

    auto modelTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, 4.0f));
    auto transform = projection * view * modelTransform;
    m_textureProgram->SetUniform("transform", transform);
    m_plane->Draw(m_textureProgram.get());

    modelTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.3f, 1.5f, 5.0f));
    transform = projection * view * modelTransform;
    m_textureProgram->SetUniform("transform", transform);
    m_plane->Draw(m_textureProgram.get());
    
    //grass
    // m_grassProgram->Use();
    // m_grassProgram->SetUniform("tex", 0);
    // m_grassTexture->Bind();
    // m_grassInstance->Bind();
    // modelTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));
    // transform = projection * view * modelTransform;
    // m_grassProgram->SetUniform("transform", transform);
    // glDrawElementsInstanced(GL_TRIANGLES, m_plane->GetIndexBuffer()->GetCount(),GL_UNSIGNED_INT, 0, m_grassPosBuffer->GetCount());

    Framebuffer::BindToDefault();
    // Resolve MSAA framebuffer to regular framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebufferMSAA->Get());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer->Get());
    glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_BLEND);

    m_postProgram->Use();
    m_postProgram->SetUniform("transform", glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f)));
    m_postProgram->SetUniform("gamma", m_gamma);
    m_framebuffer->GetColorAttachment()->Bind();
    m_postProgram->SetUniform("tex", 0);
    m_plane->Draw(m_postProgram.get());
}

void Context::ProcessInput(GLFWwindow *window)
{
    if (!m_cameraControl)
        return;
    const float cameraSpeed = 0.05f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * m_cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * m_cameraFront;

    auto cameraRight = glm::normalize(glm::cross(m_cameraUp, -m_cameraFront));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * cameraRight;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * cameraRight;

    auto cameraUp = glm::normalize(glm::cross(-m_cameraFront, cameraRight));
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * cameraUp;
}

void Context::DrawScene(const glm::mat4& view,const glm::mat4& projection, const Program* program) {
    program->Use();
    // model
    auto modelTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.5f, 0.0f));
    auto transform = projection * view * modelTransform;
    program->SetUniform("transform", transform);
    program->SetUniform("modelTransform", modelTransform);
    m_model->Draw(program);
    
    // floor
    auto floorTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 1.0f, 50.0f));
    transform = projection * view * floorTransform;
    program->SetUniform("transform", transform);
    program->SetUniform("modelTransform", floorTransform);
    m_planeMaterial->SetToProgram(program);
    m_box->Draw(program);
    //small box
    auto smallBoxTransform=glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 3.5f, 3.0f));
    transform = projection * view * smallBoxTransform;
    program->SetUniform("transform", transform);
    program->SetUniform("modelTransform", smallBoxTransform);
    m_smallBoxMaterial->SetToProgram(program);
    m_smallBox->Draw(program);
}