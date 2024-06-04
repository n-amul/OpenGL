#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include "common.h"
#include "shader.h"

CLASS_PTR(Program)
class Program
{
  public:
    static ProgramUPtr Create(const vector<ShaderPtr> &shaders);
    ~Program();
    uint32_t Get() const
    {
        return m_program;
    }
    void Use() const;
    void SetUniform(const std::string &name, int value) const;
    void SetUniform(const std::string &name, const glm::mat4 &value) const;

  private:
    Program()
    {
    }
    uint32_t m_program{0};
    bool Link(const vector<ShaderPtr> &shaders);
};

#endif