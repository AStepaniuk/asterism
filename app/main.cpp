#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <cstdio>
#include <cstdlib>

namespace {
constexpr int kDefaultWidth  = 1280;
constexpr int kDefaultHeight = 720;

void die(const char* msg) {
    std::fprintf(stderr, "[asterism] fatal: %s\n", msg);
    std::exit(EXIT_FAILURE);
}
} // namespace

int main(int, char**) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        die(SDL_GetError());
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window* window = SDL_CreateWindow(
        "Asterism", kDefaultWidth, kDefaultHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window) die(SDL_GetError());

    SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);
    if (!gl_ctx) die(SDL_GetError());
    SDL_GL_MakeCurrent(window, gl_ctx);
    SDL_GL_SetSwapInterval(1); // vsync

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        die("failed to load GL loader");
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL3_InitForOpenGL(window, gl_ctx)) die("imgui sdl3 init failed");
    if (!ImGui_ImplOpenGL3_Init("#version 330 core")) die("imgui opengl3 init failed");

    bool running = true;
    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL3_ProcessEvent(&ev);
            if (ev.type == SDL_EVENT_QUIT) running = false;
            if (ev.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                ev.window.windowID == SDL_GetWindowID(window)) {
                running = false;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // Fullscreen host window with dockspace.
        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::SetNextWindowViewport(vp->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("##DockHost", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoDocking);
        ImGui::PopStyleVar(3);
        ImGuiID dock_id = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dock_id, ImVec2(0, 0));
        ImGui::End();

        // Hello panel — replaced in M1+ by real panels.
        ImGui::Begin("Hello, Asterism");
        ImGui::Text("SDL3 + GL 3.3 + ImGui docking shell — M0");
        ImGui::Separator();
        ImGui::Text("GL renderer: %s", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
        ImGui::Text("GL version:  %s", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
        ImGui::End();

        ImGui::Render();
        int w, h;
        SDL_GetWindowSizeInPixels(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.08f, 0.09f, 0.11f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DestroyContext(gl_ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
