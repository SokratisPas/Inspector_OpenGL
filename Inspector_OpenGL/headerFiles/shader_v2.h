#ifndef SHADER_V2_H
#define SHADER_V2_H

#include<GL/glew.h>

#include<cstdio>
#include<cstdlib>
#include<fstream>
#include<string>
#include<iterator>

class shader_v2
{
private:
    unsigned ID; //Shader program ID. With this, we recognize which shader to use.

public:
    //Parse and read the vertex and fragment shader source files. Then compile both. Then link.
    shader_v2(const char *vpath, const char *fpath)
    {
        //Read the vertex shader source code from its file.
        std::ifstream fpvertex(vpath);
        if (!fpvertex)
        {
            fprintf(stderr, "Error : '%s' not found. Exiting...\n", vpath);
            exit(EXIT_FAILURE);
        }
        std::string vtemp;
        vtemp.assign( std::istreambuf_iterator<char>(fpvertex), std::istreambuf_iterator<char>() );
        const char *vsource = vtemp.c_str();
        
        int success;
        char infolog[1024];

        //Compile the vertex shader and check for errors.
        unsigned vshader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vshader, 1, &vsource, NULL);
        glCompileShader(vshader);
        glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vshader, 1024, NULL, infolog);
            fprintf(stderr, "Error while compiling '%s'.\n", vpath);
            fprintf(stderr, "%s\n", infolog);
        }
        
        //Read the fragment shader source code from its file.
        std::ifstream fpfragment(fpath);
        if (!fpfragment)
        {
            fprintf(stderr, "Error : '%s' not found. Exiting...\n", fpath);
            exit(EXIT_FAILURE);
        }
        std::string ftemp;
        ftemp.assign( std::istreambuf_iterator<char>(fpfragment), std::istreambuf_iterator<char>() );
        const char *fsource = ftemp.c_str();
        
        //Compile the fragment shader and check for errors.
        unsigned fshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fshader, 1, &fsource, NULL);
        glCompileShader(fshader);
        glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fshader, 1024, NULL, infolog);
            fprintf(stderr, "Error while compiling '%s'.\n", fpath);
            fprintf(stderr, "%s\n", infolog);
        }
        
        //Handle attach and linking.
        ID = glCreateProgram();
        glAttachShader(ID, vshader);
        glAttachShader(ID, fshader);
        glLinkProgram(ID);
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(ID, 1024, NULL, infolog);
            fprintf(stderr, "Error while linking shader program ('%s' || '%s').\n", vpath, fpath);
            fprintf(stderr, "%s\n", infolog);
        }
        
        //We no longer need the vshader and fshader, so let's delete them from now.
        //We DO need however the ID, which will be kept for deletion in the destructor.
        glDeleteShader(vshader);
        glDeleteShader(fshader);
    }

    //Delete the shader.
    ~shader_v2()
    {
        glDeleteProgram(ID);
    }
    
    //Activate the current shader.
    void use()
    {
        glUseProgram(ID);
    }

    //The following member functions are used to pass uniform variables to the shaders from the main code.

    //Pass to the currently active shader 1 int.
    void pass_uniform_int(const char *name, int value)
    {
        unsigned ulocation = glGetUniformLocation(ID, name);
        glUniform1i(ulocation, value);
    }
    
    //Pass to the currently active shader 1 float.
    void pass_uniform_float(const char *name, float value)
    {
        unsigned ulocation = glGetUniformLocation(ID, name);
        glUniform1f(ulocation, value);
    }
    
    //Pass to the currently active shader 2 floats (uniform).
    void pass_uniform_vec2(const char *name, float x, float y)
    {
        unsigned ulocation = glGetUniformLocation(ID, name);
        glUniform2f(ulocation, x,y);
    }
        
    //Pass to the currently active shader 3 floats (uniform).
    void pass_uniform_vec3(const char *name, float x, float y, float z)
    {
        unsigned ulocation = glGetUniformLocation(ID, name);
        glUniform3f(ulocation, x,y,z);
    }

    //Pass to the currently active shader 1 vector of 3 floats (uniform).
    void pass_uniform_vec3(const char *name, glm::vec3 &v)
    {
        unsigned location = glGetUniformLocation(ID, name);
        glUniform3fv(location, 1, &v[0]);
    }
    
    //Pass to the currently active shader 4 floats (uniform).
    void pass_uniform_vec4(const char *name, float x, float y, float z, float w)
    {
        unsigned ulocation = glGetUniformLocation(ID, name);
        glUniform4f(ulocation, x,y,z,w);
    }

    //Pass to the currently active shader 1 4x4 float matrix (uniform).
    void pass_uniform_mat4(const char *name, glm::mat4 &m)
    {
        unsigned location = glGetUniformLocation(ID, name);
        glUniformMatrix4fv(location, 1, GL_FALSE, &m[0][0]);
    }

    //Pass to the currently active shader 1 3x3 float matrix (uniform).
    void pass_uniform_mat3(const char* name, glm::mat3& m)
    {
        unsigned location = glGetUniformLocation(ID, name);
        glUniformMatrix3fv(location, 1, GL_FALSE, &m[0][0]);
    }

    //Pass to the currently active shader 1 bool.
    void pass_uniform_bool(const char* name, bool b)
    {
        unsigned location = glGetUniformLocation(ID, name);
        glUniform1i(location, b ? 1 : 0);
    }
};

#endif