#include "Program.h"

ProgramUPtr Program::Create(const vector<ShaderPtr> &shaders)
{
    auto program = ProgramUPtr(new Program());
    if (!program->Link(shaders))
    {
        return nullptr;
    }
    return move(program);
}
Program::~Program()
{
    if (m_program)
    {
        glDeleteProgram(m_program);
    }
}
void Program::Use() const
{
    glUseProgram(m_program);
}
bool Program::Link(const vector<ShaderPtr> &shaders)
{
    m_program = glCreateProgram();
    for (auto &sh : shaders)
    {
        glAttachShader(m_program, sh->Get());
    }
    glLinkProgram(m_program);

    int32_t success = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infolog[1024];
        glGetProgramInfoLog(m_program, 1024, nullptr, infolog);

        SPDLOG_ERROR("failed to link program {}", infolog);

        return false;
    }
    return true;
}
