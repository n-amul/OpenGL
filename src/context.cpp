#include "context.h"
#include "image.h"

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
bool Context::Init()
{
    float vertices[] = {
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,   // top right, red,textcord(s,t)
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right, green
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left, blue
        -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left, yellow
    };
    uint32_t indices[] = {
        0, 1, 3,
        1, 2, 3};

    // vao
    m_vertexLayout = VertexLayout::Create();
    // vertex buffer
    m_vertexBuffer = Buffer::CreateWithData(GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertices, sizeof(float) * 32);
    // vao setting
    m_vertexLayout->SetAttrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);
    m_vertexLayout->SetAttrib(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, sizeof(float) * 3);
    m_vertexLayout->SetAttrib(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, sizeof(float) * 6);
    // element buffer
    m_indexBuffer = Buffer::CreateWithData(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices, sizeof(uint32_t) * 6);

    // create shaders
    ShaderPtr vertShader = Shader::CreateFromFile("../shader/simplevs.txt", GL_VERTEX_SHADER);
    ShaderPtr fragShader = Shader::CreateFromFile("../shader/simplefs.txt", GL_FRAGMENT_SHADER);
    if (!vertShader || !fragShader)
    {
        return false;
    }

    SPDLOG_INFO("vertexShader ID:{}", vertShader->Get());
    SPDLOG_INFO("fragmentShader ID:{}", fragShader->Get());
    // create program
    m_program = Program::Create({fragShader, vertShader});
    if (!m_program)
    {
        return false;
    }
    SPDLOG_INFO("program ID: {}", m_program->Get());
    glClearColor(0.1f, 0.2f, 0.3f, 0.0f);

    // read image
    auto image = Image::Load("../image/wall.jpg");
    auto image2 = Image::Load("../image/container.jpg");

    if (!image || !image2)
    {
        return false;
    }
    SPDLOG_INFO("image: {}x{}. {} channels", image->GetWidth(), image->GetHeight(), image->GetChannelCount());
    //
    m_texture = Texture::CreateFromImage(image.get());
    m_texture2 = Texture::CreateFromImage(image2.get());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture->Get());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texture2->Get());

    m_program->Use();
    glUniform1i(glGetUniformLocation(m_program->Get(), "tex"), 0);
    glUniform1i(glGetUniformLocation(m_program->Get(), "tex2"), 1);
    return true;
}
void Context::Render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    m_program->Use();
    // glUniform4f(loc, t * t, 2.0f * t * (1.0f - t), (1.0f - t) * (1.0f - t), 1.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}