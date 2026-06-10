#ifndef MESHVF_H
#define MESHVF_H

#define _CRT_SECURE_NO_WARNINGS // we use this to use sscanf

#include<GL/glew.h>

#include<cstdio>
#include<cstdlib>
#include<string>
#include<fstream>
#include<vector>

class meshvf
{
private:
    unsigned vao, vbo, ebo; //Vertex array object, vertex buffer object, element (index) buffer object.
    std::vector<float> verts; //Mesh's vertices {x1,y1,z1, x2,y2,z2, ...}.
    std::vector<unsigned> inds; //Mesh's indices {vi1,vi2,vi3, vi4,vi5,vi6, ...}.

public:
    //Load the obj file, construct the mesh vectors and do the gpu memory setup.
    meshvf(const char *obj_path) : vao(0),
                                   vbo(0),
                                   ebo(0),
                                   verts(),
                                   inds()
    {
        std::ifstream fp;
        fp.open(obj_path);
        if (!fp.is_open())
        {
            fprintf(stderr, "Error : File '%s' was not found. Exiting...\n", obj_path);
            exit(EXIT_FAILURE);
        }

        float x,y,z;
        unsigned vi1,vi2,vi3;
        std::string line;
        while (getline(fp, line))
        {
            if (line[0] == 'v' && line[1] == ' ' && line.size() >= 2) //Vertex line.
            {
                sscanf(line.c_str(), "v %f %f %f", &x,&y,&z);
                verts.push_back(x);
                verts.push_back(y);
                verts.push_back(z);
            }
            else if (line[0] == 'f' && line[1] == ' ' && line.size() >= 2) //Facet line.
            {
                sscanf(line.c_str(), "f %u %u %u", &vi1,&vi2,&vi3);
                //Obj files are 1-based. Convert to 0-based tp match C++ indexing.
                inds.push_back(vi1-1);
                inds.push_back(vi2-1);
                inds.push_back(vi3-1);
            }
        }

        //Check before uploading to GPU :
        if (verts.empty() || inds.empty())
        {
            fprintf(stderr, "Error : File '%s' contains no usable vertices or faces.\n", obj_path);
            exit(EXIT_FAILURE);
        }

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), &verts[0], GL_STATIC_DRAW);

        glGenBuffers(1, &ebo); //OpenGL expects the indices stored in the ebo to reference positions in the verts[] buffer.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size()*sizeof(unsigned), &inds[0], GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        glBindVertexArray(0);
    }

    //Cleanup memory once it gets out of scope.
    ~meshvf()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }

    //Draw the mesh in the form of individual triangles.
    void draw_triangles()
    {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, inds.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    //Draw the mesh in the form of individual lines (wireframe).
    void draw_lines(const float line_width = 1.0f)
    {
        glBindVertexArray(vao);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Switch to line mode for wireframe/edge only drawing.
        glLineWidth(line_width);
        glDrawElements(GL_TRIANGLES, inds.size(), GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Restore fill mode.
        glBindVertexArray(0);
    }

    //Draw the mesh in the form of individual points (vertices).
    void draw_points(const float point_size = 1.0f)
    {
        glBindVertexArray(vao);
        glPointSize(point_size);
        glDrawArrays(GL_POINTS, 0, verts.size()/3); //Not glDrawElements()...
        glBindVertexArray(0);
    }
};

#endif
