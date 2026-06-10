// inspector.cpp
#define _CRT_SECURE_NO_WARNINGS 

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

#include <iostream>
#include <cstdio>
#include <cmath>

#include "../headerFiles/glew_setup.h"
#include "../headerFiles/shader_v2.h"
#include "../headerFiles/camera_v1.h"
#include "../headerFiles/meshvf.h"
#include "../headerFiles/meshvfnt.h"

// ===================================================================================================================================
// ===================================================================================================================================
// Inspector program (C++/OpenGL(4.1)/ImGui):
// From ImGui window you can change:
// - Background Color.
// - Render objects.
// - Light variables from Phong model (ambient, diffuse, specular, attenuation).
// - Texture.
// - Object color.
// ===================================================================================================================================
// ===================================================================================================================================


int win_width = 1600, win_height = 900;
const char* win_label = "Inspector";

camera_v1 cam;
glm::vec3 camStartPos(0.5f, -8.0f, 6.0f);    // camera's starting pos
float camStartPitch = -30.0f;                // camera's starting pitch

float xpos_previous, ypos_previous;
bool first_time_entered_the_window = true;
bool cursor_is_visible = false;

// ========= functions ========
void on_frame_update_apply_hardware(GLFWwindow* window, float frame_dt);
void key_callback(GLFWwindow* window, int key, int, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow* window, int w, int h);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(win_width, win_height, win_label, NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);

    // Register glfw callbacks.
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, mouse_scroll_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //Hide the mouse.

    glew_setup();


    // ====== MESHES =========
    // (If the meshvfnt doesn't load the .obj files make sure the are not identified as object type !!
    // If so exclude them from Build.)
    meshvfnt   plane_mesh("..\\Inspector_OpenGL\\assets\\plane10x10.obj",
                          "..\\Inspector_OpenGL\\assets\\wood_floor_2k.jpg");
    meshvfnt   chest_mesh("..\\Inspector_OpenGL\\assets\\treasure_chest.obj", 
                          "..\\Inspector_OpenGL\\assets\\treasure_chest_2k.jpg");
    meshvf    lamp_mesh("..\\Inspector_OpenGL\\assets\\uv_sphere_r1_20x20.obj");


    // ===== SHADERS ========
    shader_v2 objLightedTexture_shad("..\\Inspector_OpenGL\\vertexShaders\\objLightedTexture.vert",
                                     "..\\Inspector_OpenGL\\fragmentShaders\\objLightedTexture.frag");

    shader_v2 lamp_shad("..\\Inspector_OpenGL\\vertexShaders\\lamp_shad.vert",
                        "..\\Inspector_OpenGL\\fragmentShaders\\lamp_shad.frag");

    // ==== textures =====
    objLightedTexture_shad.use();
    objLightedTexture_shad.pass_uniform_int("tex0", 0);


    // ==== colors ===========
    glm::vec3 planeColor(1.0f); // plane color 
    glm::vec3 chestColor(1.0f); // chestt color
    glm::vec3 lampColor(1.0f);  // lamp color (same as light color)
    glm::vec3 clearColor(0.5f); // background color

    
    // ==== initialize data =======
    glm::vec3 lampPos(2.0f, 0.0f, 4.0f);    // lamp pos
    float ambientStrength = 0.1f;           // ambient component
    float diffuseStrength = 1.0f;           // diffuse component
    float specularStrength = 0.5f;          // specular component
    float k_arr[3] = {1.0f, 0.09f, 0.0032}; // attenuation 
    

    // initalize camera
    cam.position = camStartPos;
    cam.pitch = camStartPitch;

    // ===== ImGui ==========
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;   // dont create imgui.ini
    ImGui::StyleColorsDark();               // set imgui theme
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;   // round window
    style.FrameRounding = 5.0f;     // round bottons etc
    style.GrabRounding = 8.0f;      // round slide bars
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    // ImGui bools
    bool popen = true;
    bool planeTexture = true;
    bool chestTexture = true;
    bool renderChest = true;
    bool renderPlane = true;
    bool chestLit = true;
    bool planeLit = true;


    glfwGetFramebufferSize(window, &win_width, &win_height);
    glViewport(0, 0, win_width, win_height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);    

    float t0 = 0.0f, tnow, frame_dt;
    // ==============================================
    // ========== render loop =======================
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);   // change clear Color from ImGui
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        tnow = float(glfwGetTime());
        frame_dt = tnow - t0;
        t0 = tnow;

        on_frame_update_apply_hardware(window, frame_dt);


        // projection, view calculations
        glm::mat4 projection = cam.get_projection_matrix(win_width / float(win_height));
        glm::mat4 view = cam.get_view_matrix();
       
        // =========== send uniform to lamp fragment ==================
        lamp_shad.use();
        lamp_shad.pass_uniform_vec3("mesh_col", lampColor);

        // ======= send uniforms to "objLightedTexture_shad" ==========
        objLightedTexture_shad.use();        
        objLightedTexture_shad.pass_uniform_vec3("camPos", cam.position);   // send cam pos
        objLightedTexture_shad.pass_uniform_vec3("lightPos", lampPos);      // send lamp pos
        objLightedTexture_shad.pass_uniform_vec3("lightCol", lampColor);    // send light color 
        objLightedTexture_shad.pass_uniform_float("ambientStrength", ambientStrength);      // send ambient
        objLightedTexture_shad.pass_uniform_float("diffuseStrength", diffuseStrength);      // send diffuse
        objLightedTexture_shad.pass_uniform_float("specularStrength", specularStrength);    // send specular
        objLightedTexture_shad.pass_uniform_float("k1", k_arr[0]);  // send k1
        objLightedTexture_shad.pass_uniform_float("k2", k_arr[1]);  // send k2
        objLightedTexture_shad.pass_uniform_float("k3", k_arr[2]);  // send k3
        objLightedTexture_shad.pass_uniform_mat4("projection", projection); // send projection 
        objLightedTexture_shad.pass_uniform_mat4("view", view);             // send view

        // ======= plane ========== 
        // plane Texture
        // (we added a "pass_uniform_bool" !!!)

        if (renderPlane)
        {
            objLightedTexture_shad.pass_uniform_bool("isLit", planeLit);            // send if plane lit
            objLightedTexture_shad.pass_uniform_bool("showTexture", planeTexture);  // send if render texture
            objLightedTexture_shad.pass_uniform_vec3("meshColor", planeColor);      // send plain color

            glm::mat4 planeModel(1.0f);
            objLightedTexture_shad.pass_uniform_mat4("model", planeModel);
            glm::mat3 plane_transInvModel = glm::mat3(glm::transpose(glm::inverse(planeModel)));    // we calc here normals matrix 
            objLightedTexture_shad.pass_uniform_mat3("transInvModel", plane_transInvModel);
            plane_mesh.draw_triangles();            
        }

        // ====== chest =============
        // chest Texture
        if (renderChest)
        {
            objLightedTexture_shad.pass_uniform_bool("isLit", chestLit);            // send if chest lit
            objLightedTexture_shad.pass_uniform_bool("showTexture", chestTexture);  // send if render texture
            objLightedTexture_shad.pass_uniform_vec3("meshColor", chestColor);      // send chest color

            glm::mat4 chestModel(1.0f);
            objLightedTexture_shad.pass_uniform_mat4("model", chestModel);
            glm::mat3 chest_transInvModel = glm::mat3(glm::transpose(glm::inverse(chestModel)));    // we calc here normals matrix 
            objLightedTexture_shad.pass_uniform_mat3("transInvModel", chest_transInvModel);
            chest_mesh.draw_triangles();
        }

        // ========= lamp ==========
        // send view, proj 
        lamp_shad.use();
        lamp_shad.pass_uniform_mat4("projection", projection);
        lamp_shad.pass_uniform_mat4("view", view);

        // draw lamp
        glm::mat4 lampModel(1.0f);
        lampModel = glm::translate(lampModel, lampPos);
        lamp_shad.pass_uniform_mat4("model", lampModel);
        lamp_mesh.draw_triangles();


        // ==== ImGui =========
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(500.0f, 500.0f), ImGuiCond_Always); //Always same size of ImGui window!
        ImGui::Begin("GUI controls", &popen);

        if (!popen) // if close ImGui, close also glfw
            glfwSetWindowShouldClose(window, true);

        // clear Color
        ImGui::ColorEdit3("Background color", &clearColor[0]);

        ImGui::Separator();
        ImGui::Text("Light Settings");
        // light pos
        ImGui::SliderFloat3("Light possition", &lampPos.x, -20.0f, 20.0f);
        // light color
        ImGui::ColorEdit3("Light Color", &lampColor.x);

        ImGui::SliderFloat("Ambient", &ambientStrength, 0.1f, 1.0f);    // ambient
        ImGui::SliderFloat("Diffuse", &diffuseStrength, 0.1f, 3.0f);    // diffuse
        ImGui::SliderFloat("Specular", &specularStrength, 0.1f, 8.0f);  // specular
        ImGui::SliderFloat("Attenuation (k1)", &k_arr[0], 0.1f, 3.0f);          // k1
        ImGui::SliderFloat("Attenuation (k2)", &k_arr[1], 0.01f, 0.1f);         // k2
        ImGui::SliderFloat("Attenuation (k3)", &k_arr[2], 0.0020f, 0.0050f);    // k3

        // imgui chest
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Chest"))
        {
            ImGui::Checkbox("Render Chest", &renderChest);      // render chest
            ImGui::Checkbox("Chest Texture", &chestTexture);    // chest Texture
            ImGui::Checkbox("Chest Lit", &chestLit);            // chest lit
            ImGui::ColorEdit3("Chest Color", &chestColor.x);    // chest color
        }

        // imgui plane
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Floor"))
        {
            ImGui::Checkbox("Render Floor", &renderPlane);      // render plain
            ImGui::Checkbox("Floor Texture", &planeTexture);    // plain texture
            ImGui::Checkbox("Floor Lit", &planeLit);            // plain lit
            ImGui::ColorEdit3("Plane Color", &planeColor.x);    // plain color
        }

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

// ========= functions =============
// ---------------------------------------------
// take input from user (per frame)
void on_frame_update_apply_hardware(GLFWwindow* window, float frame_dt)
{
    // camera movement    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cam.translate(cam.front, frame_dt);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cam.translate(-cam.front, frame_dt);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cam.translate(cam.right, frame_dt);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cam.translate(-cam.right, frame_dt);
}

// ---------------------------------------------
//For discrete keyboard events.
void key_callback(GLFWwindow* window, int key, int, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);
}

// ---------------------------------------------
//When a mouse button is pressed, do the following :
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    //Toggle cursor visibility via the mouse right click.
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        cursor_is_visible = !cursor_is_visible;
        if (cursor_is_visible)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            first_time_entered_the_window = true;
        }
    }
}

// ---------------------------------------------
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (cursor_is_visible)
        return;

    if (first_time_entered_the_window)
    {
        xpos_previous = xpos;
        ypos_previous = ypos;
        first_time_entered_the_window = false;
    }

    float xoffset = float(xpos) - xpos_previous;
    float yoffset = float(ypos) - ypos_previous;

    xpos_previous = float(xpos);
    ypos_previous = float(ypos);

    cam.rotate(xoffset, yoffset);
}

// ---------------------------------------------
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (cursor_is_visible)
        return;

    cam.zoom(float(yoffset));
}

// ---------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    win_width = w;
    win_height = h;
    glViewport(0, 0, w, h);
}