#ifndef MESHVFNT_H
#define MESHVFNT_H

#include <GL/glew.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>

#include "stb_image.h"

class meshvfnt
{
private:
    unsigned vao, vbo, ebo;
    unsigned tex_id;

    std::vector<float> verts;   // x y z
    std::vector<float> uvs;     // u v
    std::vector<float> norms;   // nx ny nz

    std::vector<unsigned> inds;

    // x y z u v nx ny nz
    std::vector<float> verts_attr_buff;

public:

    meshvfnt(const char* obj_path, const char* img_path)
        : vao(0), vbo(0), ebo(0), tex_id(0)
    {
        std::ifstream fp(obj_path);

        if (!fp.is_open())
        {
            fprintf(stderr, "Error: file '%s' not found\n", obj_path);
            exit(EXIT_FAILURE);
        }

        float x, y, z;
        float u, v;
        float nx, ny, nz;

        unsigned vi1, ti1, ni1;
        unsigned vi2, ti2, ni2;
        unsigned vi3, ti3, ni3;

        std::string line;

        while (getline(fp, line))
        {
            if (line.size() >= 2 && line[0] == 'v' && line[1] == ' ')
            {
                sscanf(line.c_str(), "v %f %f %f", &x, &y, &z);
                verts.push_back(x);
                verts.push_back(y);
                verts.push_back(z);
            }

            else if (line.size() >= 3 &&
                line[0] == 'v' &&
                line[1] == 't')
            {
                sscanf(line.c_str(), "vt %f %f", &u, &v);
                uvs.push_back(u);
                uvs.push_back(v);
            }

            else if (line.size() >= 3 &&
                line[0] == 'v' &&
                line[1] == 'n')
            {
                sscanf(line.c_str(), "vn %f %f %f", &nx, &ny, &nz);
                norms.push_back(nx);
                norms.push_back(ny);
                norms.push_back(nz);
            }

            else if (line.size() >= 2 && line[0] == 'f')
            {
                int count = sscanf(line.c_str(),
                    "f %u/%u/%u %u/%u/%u %u/%u/%u",
                    &vi1, &ti1, &ni1,
                    &vi2, &ti2, &ni2,
                    &vi3, &ti3, &ni3);

                if (count != 9)
                    continue;

                vi1--; ti1--; ni1--;
                vi2--; ti2--; ni2--;
                vi3--; ti3--; ni3--;

                // ==== vertex 1 =====
                verts_attr_buff.push_back(verts[3 * vi1 + 0]);
                verts_attr_buff.push_back(verts[3 * vi1 + 1]);
                verts_attr_buff.push_back(verts[3 * vi1 + 2]);

                verts_attr_buff.push_back(uvs[2 * ti1 + 0]);
                verts_attr_buff.push_back(uvs[2 * ti1 + 1]);

                verts_attr_buff.push_back(norms[3 * ni1 + 0]);
                verts_attr_buff.push_back(norms[3 * ni1 + 1]);
                verts_attr_buff.push_back(norms[3 * ni1 + 2]);

                inds.push_back(verts_attr_buff.size() / 8 - 1);

                // ===== vertex 2 ======
                verts_attr_buff.push_back(verts[3 * vi2 + 0]);
                verts_attr_buff.push_back(verts[3 * vi2 + 1]);
                verts_attr_buff.push_back(verts[3 * vi2 + 2]);

                verts_attr_buff.push_back(uvs[2 * ti2 + 0]);
                verts_attr_buff.push_back(uvs[2 * ti2 + 1]);

                verts_attr_buff.push_back(norms[3 * ni2 + 0]);
                verts_attr_buff.push_back(norms[3 * ni2 + 1]);
                verts_attr_buff.push_back(norms[3 * ni2 + 2]);

                inds.push_back(verts_attr_buff.size() / 8 - 1);

                // ===== vertex 3 =====
                verts_attr_buff.push_back(verts[3 * vi3 + 0]);
                verts_attr_buff.push_back(verts[3 * vi3 + 1]);
                verts_attr_buff.push_back(verts[3 * vi3 + 2]);

                verts_attr_buff.push_back(uvs[2 * ti3 + 0]);
                verts_attr_buff.push_back(uvs[2 * ti3 + 1]);

                verts_attr_buff.push_back(norms[3 * ni3 + 0]);
                verts_attr_buff.push_back(norms[3 * ni3 + 1]);
                verts_attr_buff.push_back(norms[3 * ni3 + 2]);

                inds.push_back(verts_attr_buff.size() / 8 - 1);
            }
        }

        if (verts.empty() || uvs.empty() || norms.empty() ||
            verts_attr_buff.empty() || inds.empty())
        {
            fprintf(stderr, "Error: invalid OBJ data in '%s'\n", obj_path);
            exit(EXIT_FAILURE);
        }


        // ========== buffers ============
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts_attr_buff.size() * sizeof(float), verts_attr_buff.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned), inds.data(), GL_STATIC_DRAW);

        // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // uv
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // normal
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);

        
        // ======= Texture loading ============
        glGenTextures(1, &tex_id);
        glBindTexture(GL_TEXTURE_2D, tex_id);

        // we use GL_REPEAT to cover the hole plain(floor)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // GL_LINEAR for interpolation value in the pixels
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int w, h, ch;
        stbi_set_flip_vertically_on_load(true);

        unsigned char* data = stbi_load(img_path, &w, &h, &ch, 0);

        if (!data)
        {
            fprintf(stderr, "Error: cannot load texture '%s'\n", img_path);
            exit(EXIT_FAILURE);
        }

        unsigned format;
        if (ch == 1) format = GL_RED;
        else if (ch == 3) format = GL_RGB;
        else if (ch == 4) format = GL_RGBA;
        else
        {
            fprintf(stderr, "Unsupported image format\n");
            exit(EXIT_FAILURE);
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0,
            format, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    ~meshvfnt()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteTextures(1, &tex_id);
    }

    void draw_triangles()
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glBindVertexArray(vao);

        glDrawElements(GL_TRIANGLES, inds.size(), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

#endif
