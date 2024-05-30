#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "common.h"
#include "shader.h"
#include "program.h"
#include "buffer.h"
#include "vertex_layout.h"
#include "texture.h"

CLASS_PTR(Context)

class Context
{
public:
    ~Context();
    static ContextUPtr Create();
    void Render();

private:
    Context(){};
    bool Init();
    VertexLayoutUPtr m_vertexLayout;
    BufferUPtr m_indexBuffer;
    BufferUPtr m_vertexBuffer;

    ProgramUPtr m_program;
    TextureUPtr m_texture;
    TextureUPtr m_texture2;
};
#endif