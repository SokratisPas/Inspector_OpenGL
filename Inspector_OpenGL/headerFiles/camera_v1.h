#ifndef CAMERA_V1_H
#define CAMERA_V1_H

#include<GL/glew.h>
#include<glm.hpp>
#include<gtc/matrix_transform.hpp>

#include<cmath>

class camera_v1
{
public:
    glm::vec3 position; //Camera's position in world coordinates.
    glm::vec3 right, front, up; //Camera's local basis relative to world.
    glm::vec3 world_up; //World up direction.
    float yaw, pitch; //Camera's right-left (yaw) and up-down (pitch).
    float fov; //Camera's field of view.
    float velocity; //Camera's velocity MAGNITUDE.
    float mouse_sensitivity; //Multiplication factor.
    float zoom_sensitivity; //Multiplication factor.
    float znear, zfar;

    camera_v1() : position(glm::vec3(0.0f,0.0f,0.0f)),
                  right(1.0f,0.0f,0.0f),
                  front(0.0f,1.0f,0.0f),
                  up(0.0f,0.0f,1.0f),
                  world_up(0.0f,0.0f,1.0f),
                  yaw(90.0f),
                  pitch(0.0f),
                  fov(60.0f),
                  velocity(10.0f),
                  mouse_sensitivity(0.1f),
                  zoom_sensitivity(1.0f),
                  znear(0.01f),
                  zfar(1000.0f)
    {
        update_local_basis();
    }

    //Update camera's local basis vectors relative to world.
    void update_local_basis()
    {
        //Geographic spherical coordinates.
        front = glm::normalize(glm::vec3(cos(glm::radians(pitch))*cos(glm::radians(yaw)),
                                         cos(glm::radians(pitch))*sin(glm::radians(yaw)),
                                         sin(glm::radians(pitch))));
        right = glm::normalize(glm::cross(front, world_up));
        up    = glm::normalize(glm::cross(right, front));
    }

    //Update the camera's position.
    void translate(glm::vec3 direction, float frame_dt)
    {
        position += velocity*direction*frame_dt;
        
        //No need to update basis here!
    }
  
    //Update camera's orientation.
    void rotate(float mouse_dx, float mouse_dy)
    {
        yaw   -= mouse_dx*mouse_sensitivity;
        pitch -= mouse_dy*mouse_sensitivity;

        if (pitch >= 89.0f)
            pitch = 89.0f;
        else if (pitch <= -89.0f)
            pitch = -89.0f;

        if (yaw > 360.0f)
            yaw -= 360.0f;
        else if (yaw < -360.0f)
            yaw += 360.0f;

        update_local_basis();
    }

    //Camera zoom effect.
    void zoom(float delta_scroll)
    {
        fov -= zoom_sensitivity*delta_scroll;

        //Bound the fov :
        if (fov <= 1.0f)
            fov = 1.0f;
        else if (fov >= 179.0f)
            fov = 179.0f;
    }

    //Compute internally projection matrix.
    glm::mat4 get_projection_matrix(float window_aspect_ratio)
    {
        return glm::perspective(glm::radians(fov), window_aspect_ratio, znear, zfar);
    }

    //Compute internally view matrix.
    glm::mat4 get_view_matrix()
    {
        return glm::lookAt(position, position + front, up);
    }
};

#endif
