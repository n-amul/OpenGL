#include "context.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <spdlog/spdlog.h>

using namespace std;

void onFramebufferSizeChange(GLFWwindow *window, int width, int height)
{
    SPDLOG_INFO("framebuffer size changed: ({} x {})", width, height);
    auto context = reinterpret_cast<Context *>(glfwGetWindowUserPointer(window));
    context->Reshape(width, height);
}
void onKeyEvent(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    SPDLOG_INFO("key: {}, scancode: {}, action: {}, mods: {}{}{}", key, scancode,
                action == GLFW_PRESS     ? "Pressed"
                : action == GLFW_RELEASE ? "Released"
                : action == GLFW_REPEAT  ? "Repeat"
                                         : "Unknown",
                mods & GLFW_MOD_CONTROL ? "C" : "-", mods & GLFW_MOD_SHIFT ? "S" : "-",
                mods & GLFW_MOD_ALT ? "A" : "-");
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}
void OnCursorPos(GLFWwindow *window, double x, double y)
{
    auto context = (Context *)glfwGetWindowUserPointer(window);
    context->MouseMove(x, y);
}
void OnMouseButton(GLFWwindow *window, int button, int action, int modifier)
{
    auto context = (Context *)glfwGetWindowUserPointer(window);
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    context->MouseButton(button, action, x, y);
}

//----------------------------------------------------------//

int main(int argc, char **args)
{
    // glfw init
    SPDLOG_INFO("init glfw");
    if (!glfwInit())
    {
        const char *description = nullptr;
        glfwGetError(&description);
        SPDLOG_ERROR("failed to initialize glfw: {}", description ? description : "No error description");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw 윈도우 생성
    SPDLOG_INFO("Create glfw window");
    auto window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, nullptr, nullptr);
    if (!window)
    {
        SPDLOG_ERROR("failed to create glfw window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // glad init
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        SPDLOG_ERROR("failed to initialize glad");
        glfwTerminate();
        return -1;
    }
    auto glVersion = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    SPDLOG_INFO("OpenGL context version: {}", glVersion);
    // create context
    auto context = Context::Create();
    if (!context)
    {
        SPDLOG_ERROR("failed to init context");
        glfwTerminate();
        return -1;
    }
    glfwSetWindowUserPointer(window, context.get());
    // set events
    onFramebufferSizeChange(window, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSetFramebufferSizeCallback(window, onFramebufferSizeChange);
    glfwSetKeyCallback(window, onKeyEvent);
    glfwSetCursorPosCallback(window, OnCursorPos);
    glfwSetMouseButtonCallback(window, OnMouseButton);

    // backbuffer draw
    glClearColor(0.0f, 0.1f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // glfw 루프 실행, 윈도우 close 버튼을 누르면 정상 종료
    SPDLOG_INFO("Start main loop");
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        context->ProcessInput(window);
        context->Render();
        glfwSwapBuffers(window);
    }
    context = nullptr;
    glfwTerminate();
    return 0;
}