#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if defined(__APPLE__)
#include <GLUT/GLUT.h>
#include <OpenGL/gl3.h>
#include <OpenGL/glu.h>
#else
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations" //suppress deprecation errors

//Jonathan Fabry
//Computer Graphics
//AIT, Spring 2019

const unsigned int windowWidth = 512, windowHeight = 512;
int majorVersion = 3, minorVersion = 0;

bool keyboardState[256];
int collisionWait = 0;
bool rolling = false;
bool rolling2 = false;
int view_toggle = 0;

struct mat4
{
    float m[4][4];
public:
    mat4() {}
    mat4(float m00, float m01, float m02, float m03,
         float m10, float m11, float m12, float m13,
         float m20, float m21, float m22, float m23,
         float m30, float m31, float m32, float m33)
    {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
        m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
    }
    
    mat4 operator*(const mat4& right)
    {
        mat4 result;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                result.m[i][j] = 0;
                for (int k = 0; k < 4; k++) result.m[i][j] += m[i][k] * right.m[k][j];
            }
        }
        return result;
    }
    operator float*() { return &m[0][0]; }
    
    mat4 transpose()
    {
        return mat4(
                      m[0][0], m[1][0], m[2][0], m[3][0],
                      m[0][1], m[1][1], m[2][1], m[3][1],
                      m[0][2], m[1][2], m[2][2], m[3][2],
                      m[0][3], m[1][3], m[2][3], m[3][3]);
    }
    
    mat4 operator*(float s)
    {
        return mat4(
            m[0][0]*s, m[0][1]*s, m[0][2]*s, m[0][3]*s,
            m[1][0]*s, m[1][1]*s, m[1][2]*s, m[1][3]*s,
            m[2][0]*s, m[2][1]*s, m[2][2]*s, m[2][3]*s,
            m[3][0]*s, m[3][1]*s, m[3][2]*s, m[3][3]*s);
    }
    
    mat4 operator+(const mat4& o) const
    {
        return mat4(
        m[0][0] + o.m[0][0], m[0][1] + o.m[0][1], m[0][2] + o.m[0][2], m[0][3] + o.m[0][3],
        m[1][0] + o.m[1][0], m[1][1] + o.m[1][1], m[1][2] + o.m[1][2], m[1][3] + o.m[1][3],
        m[2][0] + o.m[2][0], m[2][1] + o.m[2][1], m[2][2] + o.m[2][2], m[2][3] + o.m[2][3],
        m[3][0] + o.m[3][0], m[3][1] + o.m[3][1], m[3][2] + o.m[3][2], m[3][3] + o.m[3][3]);
    }
    
    mat4 mul(const mat4& o) const
    {
        mat4 product;
        
        for (int r=0;r<4;r++)
            for (int c=0;c<4;c++)
                product.m[r][c] =
                m[r][0] * o.m[0][c] +
                m[r][1] * o.m[1][c] +
                m[r][2] * o.m[2][c] +
                m[r][3] * o.m[3][c];
        
        return product;
    }
    
    mat4 operator*(const mat4& o) const
    {
        return mul(o);
    }
};


struct vec4
{
    float v[4];
    
    vec4(float x = 0, float y = 0, float z = 0, float w = 1)
    {
        v[0] = x; v[1] = y; v[2] = z; v[3] = w;
    }
    
    vec4 operator*(const mat4& mat)
    {
        vec4 result;
        for (int j = 0; j < 4; j++)
        {
            result.v[j] = 0;
            for (int i = 0; i < 4; i++) result.v[j] += v[i] * mat.m[i][j];
        }
        return result;
    }
    
    vec4 operator+(const vec4& vec)
    {
        vec4 result(v[0] + vec.v[0], v[1] + vec.v[1], v[2] + vec.v[2], v[3] + vec.v[3]);
        return result;
    }
};

struct vec2
{
    float x, y;
    
    vec2(float x = 0.0, float y = 0.0) : x(x), y(y) {}
    
    vec2 operator+(const vec2& v)
    {
        return vec2(x + v.x, y + v.y);
    }
    
    vec2 operator*(float s)
    {
        return vec2(x * s, y * s);
    }
    
};

struct vec3
{
    float x, y, z;
    
    vec3(float x = 0.0, float y = 0.0, float z = 0.0) : x(x), y(y), z(z) {}
    
    static vec3 random() { return vec3(((float)rand() / RAND_MAX) * 2 - 1, ((float)rand() / RAND_MAX) * 2 - 1, ((float)rand() / RAND_MAX) * 2 - 1); }
    
    vec3 operator+(const vec3& v) { return vec3(x + v.x, y + v.y, z + v.z); }
    
    vec3 operator-(const vec3& v) { return vec3(x - v.x, y - v.y, z - v.z); }
    
    vec3 operator*(float s) { return vec3(x * s, y * s, z * s); }
    
    vec3 operator/(float s) { return vec3(x / s, y / s, z / s); }
    
    float length() { return sqrt(x * x + y * y + z * z); }
    
    vec3 normalize() { return *this / length(); }
    
    vec3 cross(const vec3& operand) const
    {
        return vec3(
                    y * operand.z - z * operand.y,
                    z * operand.x - x * operand.z,
                    x * operand.y - y * operand.x);
    }
    
    float dot(const vec3& operand) const
    {
        return x * operand.x + y * operand.y + z * operand.z;
    }
    
    void print() { printf("%f \t %f \t %f \n", x, y, z); }
};

vec3 cross(const vec3& a, const vec3& b)
{
    return vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x );
}



class Geometry
{
protected:
    unsigned int vao;
    
public:
    Geometry()
    {
        glGenVertexArrays(1, &vao);
    }

    virtual void Draw() = 0;
};


class TexturedQuad : public Geometry
{
    unsigned int vbo[3];
    
public:
    TexturedQuad()
    {
        glBindVertexArray(vao);
        glGenBuffers(3, vbo);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        static float vertexCoords[] = {-1, 0, -1,        1, 0, -1,        -1, 0, 1,        1, 0, 1};
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        static float vertexTexCoords[] = {0, 0,        10, 0,        0, 10,        10, 10};
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexTexCoords), vertexTexCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
        static float vertexNormals[] = {0, 1, 0,    0, 1, 0,    0, 1, 0,    0, 1, 0};
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexNormals), vertexNormals, GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    }
    
    void Draw()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
    }
};

class InfiniteTexturedQuad : public Geometry
{
    unsigned int vbo[3];
    
public:
    InfiniteTexturedQuad()
    {
        glBindVertexArray(vao);
        glGenBuffers(3, vbo);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        static float vertexCoords[] = {0, 0, 0, 1,        -1, 0, -1, 0,        1, 0, -1, 0,        1, 0, 1, 0,        -1, 0, 1, 0,        -1, 0, -1, 0};
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        static float vertexTexCoords[] = {5, 5,        0, 0,        10, 0,        10, 10,        0, 10,        0, 0};
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexTexCoords), vertexTexCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
        static float vertexNormals[] = {0, 1, 0,    0, 1, 0,    0, 1, 0,    0, 1, 0,    0, 1, 0,    0, 1, 0};
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexNormals), vertexNormals, GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    }
    
    void Draw()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
    }
};



class   PolygonalMesh : public Geometry
{
    struct  Face
    {
        int       positionIndices[4];
        int       normalIndices[4];
        int       texcoordIndices[4];
        bool      isQuad;
    };
    
    std::vector<std::string*> rows;
    std::vector<vec3*> positions;
    std::vector<std::vector<Face*>> submeshFaces;
    std::vector<vec3*> normals;
    std::vector<vec2*> texcoords;
    
    int nTriangles;
    
public:
    PolygonalMesh(const char *filename);
    ~PolygonalMesh();
    
    void Draw();
};


PolygonalMesh::PolygonalMesh(const char *filename)
{
    std::fstream file(filename);
    if(!file.is_open())
    {
        return;
    }
    
    char buffer[256];
    while(!file.eof())
    {
        file.getline(buffer,256);
        rows.push_back(new std::string(buffer));
    }
    
    submeshFaces.push_back(std::vector<Face*>());
    std::vector<Face*>* faces = &submeshFaces.at(submeshFaces.size()-1);
    
    for(int i = 0; i < rows.size(); i++)
    {
        if(rows[i]->empty() || (*rows[i])[0] == '#')
            continue;
        else if((*rows[i])[0] == 'v' && (*rows[i])[1] == ' ')
        {
            float tmpx,tmpy,tmpz;
            sscanf(rows[i]->c_str(), "v %f %f %f" ,&tmpx,&tmpy,&tmpz);
            positions.push_back(new vec3(tmpx,tmpy,tmpz));
        }
        else if((*rows[i])[0] == 'v' && (*rows[i])[1] == 'n')
        {
            float tmpx,tmpy,tmpz;
            sscanf(rows[i]->c_str(), "vn %f %f %f" ,&tmpx,&tmpy,&tmpz);
            normals.push_back(new vec3(tmpx,tmpy,tmpz));
        }
        else if((*rows[i])[0] == 'v' && (*rows[i])[1] == 't')
        {
            float tmpx,tmpy;
            sscanf(rows[i]->c_str(), "vt %f %f" ,&tmpx,&tmpy);
            texcoords.push_back(new vec2(tmpx,tmpy));
        }
        else if((*rows[i])[0] == 'f')
        {
            if(count(rows[i]->begin(),rows[i]->end(), ' ') == 3)
            {
                Face* f = new Face();
                f->isQuad = false;
                sscanf(rows[i]->c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
                       &f->positionIndices[0], &f->texcoordIndices[0], &f->normalIndices[0],
                       &f->positionIndices[1], &f->texcoordIndices[1], &f->normalIndices[1],
                       &f->positionIndices[2], &f->texcoordIndices[2], &f->normalIndices[2]);
                faces->push_back(f);
            }
            else
            {
                Face* f = new Face();
                f->isQuad = true;
                sscanf(rows[i]->c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                       &f->positionIndices[0], &f->texcoordIndices[0], &f->normalIndices[0],
                       &f->positionIndices[1], &f->texcoordIndices[1], &f->normalIndices[1],
                       &f->positionIndices[2], &f->texcoordIndices[2], &f->normalIndices[2],
                       &f->positionIndices[3], &f->texcoordIndices[3], &f->normalIndices[3]);
                faces->push_back(f);
            }
        }
        else if((*rows[i])[0] == 'g')
        {
            if(faces->size() > 0)
            {
                submeshFaces.push_back(std::vector<Face*>());
                faces = &submeshFaces.at(submeshFaces.size()-1);
            }
        }
    }
    
    int numberOfTriangles = 0;
    for(int iSubmesh=0; iSubmesh<submeshFaces.size(); iSubmesh++)
    {
        std::vector<Face*>& faces = submeshFaces.at(iSubmesh);
        
        for(int i=0;i<faces.size();i++)
        {
            if(faces[i]->isQuad) numberOfTriangles += 2;
            else numberOfTriangles += 1;
        }
    }
    
    nTriangles = numberOfTriangles;
    
    float *vertexCoords = new float[numberOfTriangles * 9];
    float *vertexTexCoords = new float[numberOfTriangles * 6];
    float *vertexNormalCoords = new float[numberOfTriangles * 9];
    
    
    int triangleIndex = 0;
    for(int iSubmesh=0; iSubmesh<submeshFaces.size(); iSubmesh++)
    {
        std::vector<Face*>& faces = submeshFaces.at(iSubmesh);
        
        for(int i=0;i<faces.size();i++)
        {
            if(faces[i]->isQuad)
            {
                vertexTexCoords[triangleIndex * 6] =     texcoords[faces[i]->texcoordIndices[0]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 1] = 1-texcoords[faces[i]->texcoordIndices[0]-1]->y;
                
                vertexTexCoords[triangleIndex * 6 + 2] = texcoords[faces[i]->texcoordIndices[1]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 3] = 1-texcoords[faces[i]->texcoordIndices[1]-1]->y;
                
                vertexTexCoords[triangleIndex * 6 + 4] = texcoords[faces[i]->texcoordIndices[2]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 5] = 1-texcoords[faces[i]->texcoordIndices[2]-1]->y;
                
                
                vertexCoords[triangleIndex * 9] =     positions[faces[i]->positionIndices[0]-1]->x;
                vertexCoords[triangleIndex * 9 + 1] = positions[faces[i]->positionIndices[0]-1]->y;
                vertexCoords[triangleIndex * 9 + 2] = positions[faces[i]->positionIndices[0]-1]->z;
                
                vertexCoords[triangleIndex * 9 + 3] = positions[faces[i]->positionIndices[1]-1]->x;
                vertexCoords[triangleIndex * 9 + 4] = positions[faces[i]->positionIndices[1]-1]->y;
                vertexCoords[triangleIndex * 9 + 5] = positions[faces[i]->positionIndices[1]-1]->z;
                
                vertexCoords[triangleIndex * 9 + 6] = positions[faces[i]->positionIndices[2]-1]->x;
                vertexCoords[triangleIndex * 9 + 7] = positions[faces[i]->positionIndices[2]-1]->y;
                vertexCoords[triangleIndex * 9 + 8] = positions[faces[i]->positionIndices[2]-1]->z;
                
                
                vertexNormalCoords[triangleIndex * 9] =     normals[faces[i]->normalIndices[0]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 1] = normals[faces[i]->normalIndices[0]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 2] = normals[faces[i]->normalIndices[0]-1]->z;
                
                vertexNormalCoords[triangleIndex * 9 + 3] = normals[faces[i]->normalIndices[1]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 4] = normals[faces[i]->normalIndices[1]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 5] = normals[faces[i]->normalIndices[1]-1]->z;
                
                vertexNormalCoords[triangleIndex * 9 + 6] = normals[faces[i]->normalIndices[2]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 7] = normals[faces[i]->normalIndices[2]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 8] = normals[faces[i]->normalIndices[2]-1]->z;
                
                triangleIndex++;
                
                
                vertexTexCoords[triangleIndex * 6] =     texcoords[faces[i]->texcoordIndices[1]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 1] = 1-texcoords[faces[i]->texcoordIndices[1]-1]->y;
                
                vertexTexCoords[triangleIndex * 6 + 2] = texcoords[faces[i]->texcoordIndices[2]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 3] = 1-texcoords[faces[i]->texcoordIndices[2]-1]->y;
                
                vertexTexCoords[triangleIndex * 6 + 4] = texcoords[faces[i]->texcoordIndices[3]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 5] = 1-texcoords[faces[i]->texcoordIndices[3]-1]->y;
                
                
                vertexCoords[triangleIndex * 9] =     positions[faces[i]->positionIndices[1]-1]->x;
                vertexCoords[triangleIndex * 9 + 1] = positions[faces[i]->positionIndices[1]-1]->y;
                vertexCoords[triangleIndex * 9 + 2] = positions[faces[i]->positionIndices[1]-1]->z;
                
                vertexCoords[triangleIndex * 9 + 3] = positions[faces[i]->positionIndices[2]-1]->x;
                vertexCoords[triangleIndex * 9 + 4] = positions[faces[i]->positionIndices[2]-1]->y;
                vertexCoords[triangleIndex * 9 + 5] = positions[faces[i]->positionIndices[2]-1]->z;
                
                vertexCoords[triangleIndex * 9 + 6] = positions[faces[i]->positionIndices[3]-1]->x;
                vertexCoords[triangleIndex * 9 + 7] = positions[faces[i]->positionIndices[3]-1]->y;
                vertexCoords[triangleIndex * 9 + 8] = positions[faces[i]->positionIndices[3]-1]->z;
                
                
                vertexNormalCoords[triangleIndex * 9] =     normals[faces[i]->normalIndices[1]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 1] = normals[faces[i]->normalIndices[1]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 2] = normals[faces[i]->normalIndices[1]-1]->z;
                
                vertexNormalCoords[triangleIndex * 9 + 3] = normals[faces[i]->normalIndices[2]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 4] = normals[faces[i]->normalIndices[2]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 5] = normals[faces[i]->normalIndices[2]-1]->z;
                
                vertexNormalCoords[triangleIndex * 9 + 6] = normals[faces[i]->normalIndices[3]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 7] = normals[faces[i]->normalIndices[3]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 8] = normals[faces[i]->normalIndices[3]-1]->z;
                
                triangleIndex++;
            }
            else
            {
                vertexTexCoords[triangleIndex * 6] =     texcoords[faces[i]->texcoordIndices[0]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 1] = 1-texcoords[faces[i]->texcoordIndices[0]-1]->y;
                
                vertexTexCoords[triangleIndex * 6 + 2] = texcoords[faces[i]->texcoordIndices[1]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 3] = 1-texcoords[faces[i]->texcoordIndices[1]-1]->y;
                
                vertexTexCoords[triangleIndex * 6 + 4] = texcoords[faces[i]->texcoordIndices[2]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 5] = 1-texcoords[faces[i]->texcoordIndices[2]-1]->y;
                
                vertexCoords[triangleIndex * 9] =     positions[faces[i]->positionIndices[0]-1]->x;
                vertexCoords[triangleIndex * 9 + 1] = positions[faces[i]->positionIndices[0]-1]->y;
                vertexCoords[triangleIndex * 9 + 2] = positions[faces[i]->positionIndices[0]-1]->z;
                
                vertexCoords[triangleIndex * 9 + 3] = positions[faces[i]->positionIndices[1]-1]->x;
                vertexCoords[triangleIndex * 9 + 4] = positions[faces[i]->positionIndices[1]-1]->y;
                vertexCoords[triangleIndex * 9 + 5] = positions[faces[i]->positionIndices[1]-1]->z;
                
                vertexCoords[triangleIndex * 9 + 6] = positions[faces[i]->positionIndices[2]-1]->x;
                vertexCoords[triangleIndex * 9 + 7] = positions[faces[i]->positionIndices[2]-1]->y;
                vertexCoords[triangleIndex * 9 + 8] = positions[faces[i]->positionIndices[2]-1]->z;
                
                
                vertexNormalCoords[triangleIndex * 9] =     normals[faces[i]->normalIndices[0]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 1] = normals[faces[i]->normalIndices[0]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 2] = normals[faces[i]->normalIndices[0]-1]->z;
                
                vertexNormalCoords[triangleIndex * 9 + 3] = normals[faces[i]->normalIndices[1]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 4] = normals[faces[i]->normalIndices[1]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 5] = normals[faces[i]->normalIndices[1]-1]->z;
                
                vertexNormalCoords[triangleIndex * 9 + 6] = normals[faces[i]->normalIndices[2]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 7] = normals[faces[i]->normalIndices[2]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 8] = normals[faces[i]->normalIndices[2]-1]->z;
                
                triangleIndex++;
            }
        }
    }
    
    glBindVertexArray(vao);
    
    unsigned int vbo[3];
    glGenBuffers(3, &vbo[0]);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, nTriangles * 9 * sizeof(float), vertexCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, nTriangles * 6 * sizeof(float), vertexTexCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, nTriangles * 9 * sizeof(float), vertexNormalCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
    delete vertexCoords;
    delete vertexTexCoords;
    delete vertexNormalCoords;
}


void PolygonalMesh::Draw()
{
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, nTriangles * 3);
    glDisable(GL_DEPTH_TEST);
}


PolygonalMesh::~PolygonalMesh()
{
    for(unsigned int i = 0; i < rows.size(); i++) delete rows[i];
    for(unsigned int i = 0; i < positions.size(); i++) delete positions[i];
    for(unsigned int i = 0; i < submeshFaces.size(); i++)
        for(unsigned int j = 0; j < submeshFaces.at(i).size(); j++)
            delete submeshFaces.at(i).at(j);
    for(unsigned int i = 0; i < normals.size(); i++) delete normals[i];
    for(unsigned int i = 0; i < texcoords.size(); i++) delete texcoords[i];
}


class Shader
{
protected:
    unsigned int shaderProgram;
    
    void getErrorInfo(unsigned int handle)
    {
        int logLen;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen > 0)
        {
            char * log = new char[logLen];
            int written;
            glGetShaderInfoLog(handle, logLen, &written, log);
            printf("Shader log:\n%s", log);
            delete log;
        }
    }
    
    void checkShader(unsigned int shader, char * message)
    {
        int OK;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &OK);
        if (!OK)
        {
            printf("%s!\n", message);
            getErrorInfo(shader);
        }
    }
    
    void checkLinking(unsigned int program)
    {
        int OK;
        glGetProgramiv(program, GL_LINK_STATUS, &OK);
        if (!OK)
        {
            printf("Failed to link shader program!\n");
            getErrorInfo(program);
        }
    }
    
public:
    Shader()
    {
        shaderProgram = 0;
    }
    
    ~Shader()
    {
        if(shaderProgram) glDeleteProgram(shaderProgram);
    }
    
    void CompileProgram(const char *vertexSource, const char *fragmentSource)
    {
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        if (!vertexShader) { printf("Error in vertex shader creation\n"); exit(1); }
        
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);
        checkShader(vertexShader, "Vertex shader error");
        
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        if (!fragmentShader) { printf("Error in fragment shader creation\n"); exit(1); }
        
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        checkShader(fragmentShader, "Fragment shader error");
        
        shaderProgram = glCreateProgram();
        if (!shaderProgram) { printf("Error in shader program creation\n"); exit(1); }
        
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
    }
    
    void LinkProgram()
    {
        glLinkProgram(shaderProgram);
        checkLinking(shaderProgram);
    }
    
    void Run()
    {
        if(shaderProgram) glUseProgram(shaderProgram);
    }
    
    virtual void UploadM(mat4& M) { }
    
    virtual void UploadInvM(mat4& InVM) { }
    
    virtual void UploadVP(mat4& MVP) { }
    
    virtual void UploadMVP(mat4& MVP) { }
    
    virtual void UploadSamplerID() { }
    
    virtual void UploadLightAttributes(vec4& worldLightPosition, vec3& Le, vec3& La) { }
    
    virtual void UploadMaterialAttributes(vec3& ka, vec3& kd, vec3& ks, float shininess) { }
    
    virtual void UploadEyePosition(vec3& eye) { }
};

class MarbleShader : public Shader
{
public:
    MarbleShader()
    {
        const char *vertexSource = R"(
        #version 410
        precision highp float;
        in vec3 vertexPosition;
        in vec2 vertexTexCoord;
        in vec3 vertexNormal;
        uniform mat4 M, InvM, MVP;
        uniform vec3 worldEyePosition;
        uniform vec4 worldLightPosition;
        out vec2 texCoord;
        out vec3 worldNormal;
        out vec3 worldView;
        out vec3 worldLight;
        
        void main() {
            texCoord = vertexTexCoord;
            vec4 worldPosition = vec4(vertexPosition, 1) * M;
            worldLight  = worldLightPosition.xyz * worldPosition.w - worldPosition.xyz * worldLightPosition.w;
            worldView = worldEyePosition - worldPosition.xyz;
            worldNormal = (InvM * vec4(vertexNormal, 0.0)).xyz;
            gl_Position = vec4(vertexPosition, 1) * MVP;
        }
        )";
        
        const char *fragmentSource = R"(
        #version 410
        precision highp float;
        uniform sampler2D samplerUnit;
        uniform vec3 La, Le;
        uniform vec3 ka, kd, ks;
        uniform float shininess;
        in vec2 texCoord;
        in vec3 worldNormal;
        in vec3 worldView;
        in vec3 worldLight;
        out vec4 fragmentColor;
        
        float snoise(vec3 r) {
            vec3 s = vec3(7502, 22777, 4767);
            float w = 0.0;
            for(int i=0; i<16; i++) {
                w += sin( dot(s - vec3(32768, 32768, 32768),
                              r * 40.0) / 65536.0);
                s = mod(s, 32768.0) * 2.0 + floor(s / 32768.0);
            }
            return w / 32.0 + 0.5;
        }
        
        vec3 getColor(vec3 position){
            float w = position.x * 32.0
            + pow(snoise(position * 32.0), 1.0)*50.0;
            
            w = pow(sin(w)*0.5+0.5, 4);
            return vec3(0, 0, 1) * w + vec3(1, 1, 1) * (1-w);
        }
        
        void main() {
            vec3 N = normalize(worldNormal);
            vec3 V = normalize(worldView);
            vec3 L = normalize(worldLight);
            vec3 H = normalize(V + L);
            vec3 texel = getColor(vec3(texCoord,0.0));
            vec3 color = La * ka +
            Le * kd * texel * max(0.0, dot(L, N)) +
            Le * ks * pow(max(0.0, dot(H, N)), shininess);
            fragmentColor = vec4(color, 1);
        }
        )";
        
        CompileProgram(vertexSource, fragmentSource);
        
        glBindAttribLocation(shaderProgram, 0, "vertexPosition");
        glBindAttribLocation(shaderProgram, 1, "vertexTexCoord");
        glBindAttribLocation(shaderProgram, 2, "vertexNormal");
        
        glBindFragDataLocation(shaderProgram, 0, "fragmentColor");
        
        LinkProgram();
    }
    
    void UploadSamplerID()
    {
        int samplerUnit = 0;
        int location = glGetUniformLocation(shaderProgram, "samplerUnit");
        glUniform1i(location, samplerUnit);
        glActiveTexture(GL_TEXTURE0 + samplerUnit);
    }
    
    void UploadM(mat4& M)
    {
        int location = glGetUniformLocation(shaderProgram, "M");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, M);
        else printf("uniform M cannot be set\n");
    }
    
    void UploadInvM(mat4& InvM)
    {
        int location = glGetUniformLocation(shaderProgram, "InvM");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, InvM);
        else printf("uniform InvM cannot be set\n");
    }
    
    void UploadMVP(mat4& MVP)
    {
        int location = glGetUniformLocation(shaderProgram, "MVP");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, MVP);
        else printf("uniform MVP cannot be set\n");
    }
    
    void UploadLightAttributes(vec4& worldLightPosition, vec3& Le, vec3& La)
    {
        int location = glGetUniformLocation(shaderProgram, "Le");
        if (location >= 0) glUniform3fv(location,1, &Le.x);
        else printf("uniform Le cannot be set\n");
        
        location = glGetUniformLocation(shaderProgram, "La");
        if (location >= 0) glUniform3fv(location,1, &La.x);
        else printf("uniform La cannot be set\n");
        
        location = glGetUniformLocation(shaderProgram, "worldLightPosition");
        if (location >= 0) glUniform4fv(location, 1, &worldLightPosition.v[0]);
        else printf("uniform worldLightPosition cannot be set\n");
    }
    
    void UploadMaterialAttributes(vec3& ka, vec3& kd, vec3& ks, float shininess)
    {
        int location = glGetUniformLocation(shaderProgram, "ka");
        if (location >= 0) glUniform3fv(location,1, &ka.x);
        else printf("uniform ka cannot be set\n");
        
        location = glGetUniformLocation(shaderProgram, "kd");
        if (location >= 0) glUniform3fv(location,1, &kd.x);
        else printf("uniform kd cannot be set\n");
        
        location = glGetUniformLocation(shaderProgram, "ks");
        if (location >= 0) glUniform3fv(location,1, &ks.x);
        else printf("uniform ks cannot be set\n");
        
        location = glGetUniformLocation(shaderProgram, "shininess");
        if (location >= 0) glUniform1f(location, shininess);
        else printf("uniform shininess cannot be set\n");
    }
    
    void UploadEyePosition(vec3& eye)
    {
        int location = glGetUniformLocation(shaderProgram, "worldEyePosition");
        if (location >= 0) glUniform3fv(location,1, &eye.x);
        else printf("uniform worldEyePosition cannot be set\n");
    }
};

class ShadowShader : public Shader
{
public:
    ShadowShader()
    {
        const char *vertexSource = R"(
        #version 410
        precision highp float;
        
        in vec3 vertexPosition;
        in vec2 vertexTexCoord;
        in vec3 vertexNormal;
        uniform mat4 M, VP;
        uniform vec4 worldLightPosition;
        
        void main() {
            vec4 p = vec4(vertexPosition, 1) * M;
            vec3 s;
            s.y = -0.999;
            s.x = (p.x - worldLightPosition.x) / (p.y - worldLightPosition.y) * (s.y - worldLightPosition.y) + worldLightPosition.x;
            s.z = (p.z - worldLightPosition.z) / (p.y - worldLightPosition.y) * (s.y - worldLightPosition.y) + worldLightPosition.z;
            gl_Position = vec4(s, 1) * VP;
        }
        )";
        
        const char *fragmentSource = R"(
        #version 410
        precision highp float;
        
        out vec4 fragmentColor;
        
        void main()
        {
            fragmentColor = vec4(0.0, 0.1, 0.0, 1);
        }
        )";
        
        CompileProgram(vertexSource, fragmentSource);
        
        glBindAttribLocation(shaderProgram, 0, "vertexPosition");
        glBindAttribLocation(shaderProgram, 1, "vertexTexCoord");
        glBindAttribLocation(shaderProgram, 2, "vertexNormal");
        
        glBindFragDataLocation(shaderProgram, 0, "fragmentColor");
        
        LinkProgram();
    }
    
    void UploadM(mat4& M)
    {
        int location = glGetUniformLocation(shaderProgram, "M");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, M);
        else printf("uniform M cannot be set\n");
    }
    
    void UploadVP(mat4& VP)
    {
        int location = glGetUniformLocation(shaderProgram, "VP");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, VP);
        else printf("uniform VP cannot be set\n");
    }
    
    void UploadLightAttributes(vec4& worldLightPosition, vec3& Le, vec3& La)
    {
        int location = glGetUniformLocation(shaderProgram, "worldLightPosition");
        if (location >= 0) glUniform4fv(location, 1, &worldLightPosition.v[0]);
        else printf("uniform worldLightPosition cannot be set\n");
    }
};

class InfiniteQuadShader : public Shader
{
public:
    InfiniteQuadShader()
    {
        const char *vertexSource = R"(
        #version 410
        precision highp float;
        
        in vec4 vertexPosition;
        in vec2 vertexTexCoord;
        in vec3 vertexNormal;
        uniform mat4 M, InvM, MVP;
        
        out vec2 texCoord;
        out vec4 worldPosition;
        out vec3 worldNormal;
        
        void main() {
            texCoord = vertexTexCoord;
            worldPosition = vertexPosition * M;
            worldNormal = (InvM * vec4(vertexNormal, 0.0)).xyz;
            gl_Position = vertexPosition * MVP;
        }
        )";
        
        const char *fragmentSource = R"(
        #version 410
        precision highp float;
        uniform sampler2D samplerUnit;
        uniform vec3 La, Le;
        uniform vec3 ka, kd, ks;
        uniform float shininess;
        uniform vec3 worldEyePosition;
        uniform vec4 worldLightPosition;
        in vec2 texCoord;
        in vec4 worldPosition;
        in vec3 worldNormal;
        out vec4 fragmentColor;
        void main() {
            vec3 N = normalize(worldNormal);
            vec3 V = normalize(worldEyePosition * worldPosition.w - worldPosition.xyz);
            vec3 L = normalize(worldLightPosition.xyz * worldPosition.w - worldPosition.xyz * worldLightPosition.w);
            vec3 H = normalize(V + L);
            vec2 position = worldPosition.xz / worldPosition.w;
            vec2 tex = position.xy - floor(position.xy);
            vec3 texel = texture(samplerUnit, tex).xyz;
            vec3 color = La * ka + Le * kd * texel * max(0.0, dot(L, N)) + Le * ks * pow(max(0.0, dot(H, N)), shininess);
            fragmentColor = vec4(color, 1);
        }
        )";
        
        CompileProgram(vertexSource, fragmentSource);
        
        glBindAttribLocation(shaderProgram, 0, "vertexPosition");
        glBindAttribLocation(shaderProgram, 1, "vertexTexCoord");
        glBindAttribLocation(shaderProgram, 2, "vertexNormal");
        
        glBindFragDataLocation(shaderProgram, 0, "fragmentColor");
        
        LinkProgram();
    }
    
    void UploadSamplerID()
    {
        int samplerUnit = 0;
        int location = glGetUniformLocation(shaderProgram, "samplerUnit");
        glUniform1i(location, samplerUnit);
        glActiveTexture(GL_TEXTURE0 + samplerUnit);
    }
    
    void UploadM(mat4& M)
    {
        int location = glGetUniformLocation(shaderProgram, "M");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, M);
        else printf("uniform M cannot be set\n");
    }
    
    void UploadInvM(mat4& InvM)
    {
        int location = glGetUniformLocation(shaderProgram, "InvM");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, InvM);
        else printf("uniform InvM cannot be set\n");
    }
    
    void UploadMVP(mat4& MVP)
    {
        int location = glGetUniformLocation(shaderProgram, "MVP");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, MVP);
        else printf("uniform MVP cannot be set\n");
    }
    
    void UploadLightAttributes(vec4& worldLightPosition, vec3& Le, vec3& La)
    {
        int location = glGetUniformLocation(shaderProgram, "Le");
        if (location >= 0) glUniform3fv(location,1, &Le.x);
        else printf("uniform Le cannot be set\n");
        
        location = glGetUniformLocation(shaderProgram, "La");
        if (location >= 0) glUniform3fv(location,1, &La.x);
        else printf("uniform La cannot be set\n");
        
        location = glGetUniformLocation(shaderProgram, "worldLightPosition");
        if (location >= 0) glUniform4fv(location, 1, &worldLightPosition.v[0]);
        else printf("uniform worldLightPosition cannot be set\n");
    }
    
    void UploadMaterialAttributes(vec3& ka, vec3& kd, vec3& ks, float shininess)
    {
        int location = glGetUniformLocation(shaderProgram, "ka");
        if (location >= 0) glUniform3fv(location,1, &ka.x);
        else printf("uniform ka cannot be set\n");
        
        location = glGetUniformLocation(shaderProgram, "kd");
        if (location >= 0) glUniform3fv(location,1, &kd.x);
        else printf("uniform kd cannot be set\n");
        
        location = glGetUniformLocation(shaderProgram, "ks");
        if (location >= 0) glUniform3fv(location,1, &ks.x);
        else printf("uniform ks cannot be set\n");
        
        location = glGetUniformLocation(shaderProgram, "shininess");
        if (location >= 0) glUniform1f(location, shininess);
        else printf("uniform shininess cannot be set\n");
    }
    
    void UploadEyePosition(vec3& eye)
    {
        int location = glGetUniformLocation(shaderProgram, "worldEyePosition");
        if (location >= 0) glUniform3fv(location,1, &eye.x);
        else printf("uniform worldEyePosition cannot be set\n");
    }
};

class MeshShader : public Shader
{
public:
    MeshShader()
    {
        const char *vertexSource = R"(
        #version 410
        precision highp float;
        in vec3 vertexPosition;
        in vec2 vertexTexCoord;
        in vec3 vertexNormal;
        uniform mat4 M, InvM, MVP;
        uniform vec3 worldEyePosition;
        uniform vec4 worldLightPosition;
        out vec2 texCoord;
        out vec3 worldNormal;
        out vec3 worldView;
        out vec3 worldLight;
        
        void main() {
            texCoord = vertexTexCoord;
            vec4 worldPosition = vec4(vertexPosition, 1) * M;
            worldLight  = worldLightPosition.xyz * worldPosition.w - worldPosition.xyz * worldLightPosition.w;
            worldView = worldEyePosition - worldPosition.xyz;
            worldNormal = (InvM * vec4(vertexNormal, 0.0)).xyz;
            gl_Position = vec4(vertexPosition, 1) * MVP;
        }
        )";
        
        const char *fragmentSource = R"(
        #version 410
        precision highp float;
        uniform sampler2D samplerUnit;
        uniform vec3 La, Le;
        uniform vec3 ka, kd, ks;
        uniform float shininess;
        in vec2 texCoord;
        in vec3 worldNormal;
        in vec3 worldView;
        in vec3 worldLight;
        out vec4 fragmentColor;
        
        void main() {
            vec3 N = normalize(worldNormal);
            vec3 V = normalize(worldView);
            vec3 L = normalize(worldLight);
            vec3 H = normalize(V + L);
            vec3 texel = texture(samplerUnit, texCoord).xyz;
            vec3 color =
            La * ka +
            Le * kd * texel * max(0.0, dot(L, N)) +
            Le * ks * pow(max(0.0, dot(H, N)), shininess);
            fragmentColor = vec4(color, 1);
        }
        )";
        
        CompileProgram(vertexSource, fragmentSource);
        
        glBindAttribLocation(shaderProgram, 0, "vertexPosition");
        glBindAttribLocation(shaderProgram, 1, "vertexTexCoord");
        glBindAttribLocation(shaderProgram, 2, "vertexNormal");
        
        glBindFragDataLocation(shaderProgram, 0, "fragmentColor");
        
        LinkProgram();
    }
    
    void UploadSamplerID()
    {
        int samplerUnit = 0;
        int location = glGetUniformLocation(shaderProgram, "samplerUnit");
        glUniform1i(location, samplerUnit);
        glActiveTexture(GL_TEXTURE0 + samplerUnit);
    }
    
    void UploadM(mat4& M)
    {
        int location = glGetUniformLocation(shaderProgram, "M");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, M);
        else printf("uniform M cannot be set\n");
    }
    
    void UploadInvM(mat4& InvM)
    {
        int location = glGetUniformLocation(shaderProgram, "InvM");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, InvM);
        else printf("uniform InvM cannot be set\n");
    }
    
    void UploadMVP(mat4& MVP)
    {
        int location = glGetUniformLocation(shaderProgram, "MVP");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, MVP);
        else printf("uniform MVP cannot be set\n");
    }
    
    void UploadLightAttributes(vec4& worldLightPosition, vec3& Le, vec3& La)
    {
        int location = glGetUniformLocation(shaderProgram, "Le");
        if (location >= 0) glUniform3fv(location,1, &Le.x);
        else printf("uniform Le cannot be set\n");
        
        location = glGetUniformLocation(shaderProgram, "La");
        if (location >= 0) glUniform3fv(location,1, &La.x);
        else printf("uniform La cannot be set\n");
        
        location = glGetUniformLocation(shaderProgram, "worldLightPosition");
        if (location >= 0) glUniform4fv(location, 1, &worldLightPosition.v[0]);
        else printf("uniform worldLightPosition cannot be set\n");
    }
    
    void UploadMaterialAttributes(vec3& ka, vec3& kd, vec3& ks, float shininess)
    {
        int location = glGetUniformLocation(shaderProgram, "ka");
        if (location >= 0) glUniform3fv(location,1, &ka.x);
        else printf("uniform ka cannot be set\n");
        
        location = glGetUniformLocation(shaderProgram, "kd");
        if (location >= 0) glUniform3fv(location,1, &kd.x);
        else printf("uniform kd cannot be set\n");
        
        location = glGetUniformLocation(shaderProgram, "ks");
        if (location >= 0) glUniform3fv(location,1, &ks.x);
        else printf("uniform ks cannot be set\n");
        
        location = glGetUniformLocation(shaderProgram, "shininess");
        if (location >= 0) glUniform1f(location, shininess);
        else printf("uniform shininess cannot be set\n");
    }
    
    void UploadEyePosition(vec3& eye)
    {
        int location = glGetUniformLocation(shaderProgram, "worldEyePosition");
        if (location >= 0) glUniform3fv(location,1, &eye.x);
        else printf("uniform worldEyePosition cannot be set\n");
    }
};

class Camera {
    vec3  wEye, wLookat, wVup;
    float fov, asp, fp, bp;
    
    vec3 velocity;
    float angularVelocity;
    
public:
    Camera()
    {
        wEye = vec3(0.0, 0.0, 2.0);
        wLookat = vec3(0.0, 0.0, 0.0);
        wVup = vec3(0.0, 1.0, 0.0);
        fov = M_PI / 4.0; asp = 1.0; fp = 0.01; bp = 100.0;
        
        velocity = vec3(0.0, 0.0, 0.0);
        angularVelocity = 0.0;
    }
    
    void SetAspectRatio(float a) { asp = a; }
    
    vec3 GetEyePosition()
    {
        return wEye;
    }
    
    mat4 GetViewMatrix()
    {
        vec3 w = (wEye - wLookat).normalize();
        vec3 u = cross(wVup, w).normalize();
        vec3 v = cross(w, u);
        
        return
        mat4(
             1.0f,    0.0f,    0.0f,    0.0f,
             0.0f,    1.0f,    0.0f,    0.0f,
             0.0f,    0.0f,    1.0f,    0.0f,
             -wEye.x, -wEye.y, -wEye.z, 1.0f ) *
        mat4(
             u.x,  v.x,  w.x,  0.0f,
             u.y,  v.y,  w.y,  0.0f,
             u.z,  v.z,  w.z,  0.0f,
             0.0f, 0.0f, 0.0f, 1.0f );
    }
    
    mat4 GetProjectionMatrix()
    {
        float sy = 1/tan(fov/2);
        return mat4(
                    sy/asp, 0.0f,  0.0f,               0.0f,
                    0.0f,   sy,    0.0f,               0.0f,
                    0.0f,   0.0f, -(fp+bp)/(bp - fp), -1.0f,
                    0.0f,   0.0f, -2*fp*bp/(bp - fp),  0.0f);
    }
    
    vec3 GetAhead()
    {
        return (wLookat - wEye).normalize();
    }
    
    void Control()
    {
        if(keyboardState['w'])
        {
            velocity = GetAhead() * 2.0;
            return;
        }
        
        if(keyboardState['s'])
        {
            velocity = GetAhead() * (-2.0);
            return;
        }
        
        if(keyboardState['d'])
        {
            angularVelocity = 20.0;
            return;
        }
        
        if(keyboardState['a'])
        {
            angularVelocity = -20.0;
            return;
        }
        
        angularVelocity = 0.0;
        velocity = vec3(0.0, 0.0, 0.0);
    }
    
    void Move(float dt)
    {
        wEye = wEye + velocity * dt;
        wLookat = wLookat + velocity * dt;
        
        vec3 w = (wLookat - wEye);
        float l = w.length();
        w = w.normalize();
        vec3 v = wVup.normalize();
        vec3 u = cross(w, v);
        
        float alpha = (angularVelocity * dt) * M_PI / 180.0;
        
        wLookat = wEye + (w * cos(alpha) + u * sin(alpha)) * l;
    }
    
    void UploadAttributes(Shader *shader)
    {
        shader->UploadEyePosition(wEye);
    }
    
    void Helicam(vec3 AvatarPos, float yOrientation)
    {
        wEye = AvatarPos;

        float xOffest = sin(yOrientation / 180.0 * M_PI);
        float zOffest = cos(yOrientation / 180.0 * M_PI);
        
        wEye = wEye + vec3(3*xOffest, -0.1, -3*zOffest);
        wEye.y += 1;
        wLookat = AvatarPos;
        wLookat.z += 0.5;
    }
    
    void SideView(vec3 AvatarPos)
    {
        wEye = AvatarPos;
        wEye.y += 2;
        wEye.x -= 5;
        wLookat = AvatarPos;
    }
    
    void AerialView(vec3 AvatarPos, float yOrientation)
    {
        wEye = AvatarPos;
        
        float xOffest = sin(yOrientation / 180.0 * M_PI);
        float zOffest = cos(yOrientation / 180.0 * M_PI);
        wEye = wEye + vec3(3*xOffest, -0.1, -3*zOffest);
        
        wEye.y += 6.5;
        wEye.z -= 0.1;
        wLookat = AvatarPos;
    }
    
    void FrontView(vec3 AvatarPos)
    {
        wEye = AvatarPos;
        wEye.y += 0.75;
        wEye.z -= 3;
        wLookat = AvatarPos;
    }
    
    void Flyover1(vec3 AvatarPos, float yOrientation)
    {
        wEye = AvatarPos;
        
        float xOffest = sin(AvatarPos.x / 180.0 * M_PI);
        float zOffest = cos(AvatarPos.x / 180.0 * M_PI);
        wEye = (wEye.x * xOffest, wEye.y, wEye.z * zOffest);
        wEye.y += 1;
        wLookat = AvatarPos;
        wLookat.z += 0.5;

    }
    
    void Flyover2(vec3 AvatarPos, float yOrientation)
    {
        wEye = AvatarPos;
        
        float xOffest = sin(yOrientation / 180.0 * M_PI);
        float zOffest = cos(yOrientation / 180.0 * M_PI);
        wEye = vec3(wEye.x * xOffest, wEye.y * xOffest * zOffest , wEye.z * zOffest);
        wEye.y += 1;
        wLookat = AvatarPos;
        wLookat.z += 0.5;
        
    }
    
    void Flyover3(vec3 AvatarPos, float yOrientation)
    {
        wEye = AvatarPos;
        
        float xOffest = fmod(sin(yOrientation / 180.0 * M_PI), 1000);
        float zOffest = fmod(cos(yOrientation / 180.0 * M_PI), 1000);
        wEye = vec3(wEye.x * xOffest, fmax(wEye.y * 2 * xOffest * zOffest + 0.5*wEye.y, -0.5) , wEye.z * xOffest);
        wEye.y += 1;
        wLookat = AvatarPos;
        wLookat.z += 0.5;
        
    }
};

Camera camera;


extern "C" unsigned char* stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp);

class Texture
{
    unsigned int textureId;
    
public:
    Texture(const std::string& inputFileName)
    {
        unsigned char* data;
        int width; int height; int nComponents = 4;
        
        data = stbi_load(inputFileName.c_str(), &width, &height, &nComponents, 0);
        
        if(data == NULL)
        {
            return;
        }
        
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        
        if(nComponents == 3) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        if(nComponents == 4) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        delete data;
    }
    
    void Bind()
    {
        glBindTexture(GL_TEXTURE_2D, textureId);
    }
};



class Material
{
    Shader* shader;
    Texture* texture;
    
    vec3 ka, kd, ks;
    float shininess;
    
public:
    Material(Shader* s, vec3 ka, vec3 kd, vec3 ks, float shininess, Texture* t = 0) : ka(ka), kd(kd), ks(ks), shininess(shininess)
    {
        shader = s;
        texture = t;
    }
    
    Shader* GetShader() { return shader; }
    
    void UploadAttributes()
    {
        if(texture)
        {
            shader->UploadSamplerID();
            texture->Bind();
        }
        
        shader->UploadMaterialAttributes(ka, kd, ks, shininess);
    }
};

class Mesh
{
    Geometry* geometry;
    Material* material;
    
public:
    Mesh(Geometry* g, Material* m)
    {
        geometry = g;
        material = m;
    }
    
    Shader* GetShader() { return material->GetShader(); }
    
    void Draw()
    {
        material->UploadAttributes();
        geometry->Draw();
    }
};



class Light
{
    vec3 La, Le;
    vec4 worldLightPosition;
    
public:
    
    Light(vec4 worldLightPosition, vec3 Le = vec3(1.0, 1.0, 1.0), vec3 La = vec3(1.0, 1.0, 1.0)) : worldLightPosition(worldLightPosition), Le(Le), La(La)
    {
        
    }
    
    void SetPointLightSource(vec3 pos)
    {
        worldLightPosition.v[0] = pos.x;
        worldLightPosition.v[1] = pos.y;
        worldLightPosition.v[2] = pos.z;
        worldLightPosition.v[3] = 1.0;
    }
    
    void SetDirectionalLightSource(vec3 dir)
    {
        worldLightPosition.v[0] = dir.x;
        worldLightPosition.v[1] = dir.y;
        worldLightPosition.v[2] = dir.z;
        worldLightPosition.v[3] = 0.0;
    }
    
    void UploadAttributes(Shader *shader)
    {
        shader->UploadLightAttributes(worldLightPosition, Le, La);
    }
};

Light sunlight(vec4(0.0, 1.0, 2.0, 1.0), vec3(1, 1, 1), vec3(1, 1, 1));
Light pointlight(vec4(0.0, 10.0, 2.0, 1.0), vec3(2.0, 2.0, 2.0), vec3(1.5, 1.5, 1.5));

class Object
{
    Shader* shader;
    Mesh *mesh;

protected:
    vec3 position;
    vec3 scaling;
    vec3 orientation;
    vec3 velocity;
    vec3 angularV;
    
public:
    Object(Mesh *m, vec3 v, vec3 position = vec3(0.0, 0.0, 0.0), vec3 scaling = vec3(1.0, 1.0, 1.0), vec3 orientation = vec3(0.0, 0.0, 0.0), vec3 angularV = vec3(0.0, 0.0, 0.0)) : position(position), scaling(scaling), orientation(orientation), angularV(angularV)
    {
        shader = m->GetShader();
        mesh = m;
        velocity = v;
    }
    
    vec3 GetPosition() { return position; }
    
    virtual vec3 GetOrientation() { return orientation; }
    
    virtual void Draw()
    {
        shader->Run();
        
        UploadAttributes(shader);
        
        sunlight.SetDirectionalLightSource(vec3(-100.0, 300.0, 300.0));
        sunlight.UploadAttributes(shader);
        
        camera.UploadAttributes(shader);
        
        mesh->Draw();
    }
    
    virtual void DrawShadow(Shader* shadowShader)
    {
        shadowShader->Run();
        
        UploadAttributes(shadowShader);
        
        sunlight.SetDirectionalLightSource(vec3(-100.0, 300.0, 300.0));
        sunlight.UploadAttributes(shadowShader);
        
        camera.UploadAttributes(shadowShader);
        
        mesh->Draw();
    }
    
    void SpotlightDrawShadow(Shader* shadowShader)
    {
        shadowShader->Run();
        
        UploadAttributes(shadowShader);
        
        vec3 eye = camera.GetEyePosition();
        pointlight.SetPointLightSource(eye+vec3(2, 5, 2));
        pointlight.UploadAttributes(shadowShader);
        
        camera.UploadAttributes(shadowShader);
        
        mesh->Draw();
    }
    
    virtual void UploadAttributes(Shader *s)
    {
        mat4 T = mat4(
                      1.0,            0.0,            0.0,            0.0,
                      0.0,            1.0,            0.0,            0.0,
                      0.0,            0.0,            1.0,            0.0,
                      position.x,        position.y,        position.z,        1.0);
        
        mat4 InvT = mat4(
                         1.0,            0.0,            0.0,            0.0,
                         0.0,            1.0,            0.0,            0.0,
                         0.0,            0.0,            1.0,            0.0,
                         -position.x,    -position.y,    -position.z,    1.0);
        
        mat4 S = mat4(
                      scaling.x,        0.0,            0.0,            0.0,
                      0.0,            scaling.y,        0.0,            0.0,
                      0.0,            0.0,            scaling.z,        0.0,
                      0.0,            0.0,            0.0,            1.0);
        
        mat4 InvS = mat4(
                         1.0/scaling.x,    0.0,            0.0,            0.0,
                         0.0,            1.0/scaling.y,    0.0,            0.0,
                         0.0,            0.0,            1.0/scaling.z,    0.0,
                         0.0,            0.0,            0.0,            1.0);
        
        float alpha = orientation.y / 180.0 * M_PI;
        
        mat4 R = mat4(
                      cos(alpha),     0.0,            sin(alpha),     0.0,
                      0.0,            1.0,            0.0,            0.0,
                      -sin(alpha),    0.0,            cos(alpha),     0.0,
                      0.0,            0.0,            0.0,            1.0);
        
        mat4 InvR = mat4(
                         cos(alpha),     0.0,            -sin(alpha),    0.0,
                         0.0,            1.0,            0.0,            0.0,
                         sin(alpha),     0.0,            cos(alpha),     0.0,
                         0.0,            0.0,            0.0,            1.0);
        
        float zAlpha = orientation.z / 180.0 * M_PI;
        
        mat4 ZR = mat4(
                      cos(zAlpha),   -sin(zAlpha),    0.0,        0.0,
                      sin(zAlpha),   cos(zAlpha),     0.0,        0.0,
                      0.0,          0.0,            1.0,        0.0,
                      0.0,          0.0,            0.0,        1.0);
        
        mat4 InvZR = mat4(
                          cos(zAlpha),   sin(zAlpha),    0.0,        0.0,
                          -sin(zAlpha),  cos(zAlpha),     0.0,        0.0,
                          0.0,          0.0,            1.0,        0.0,
                          0.0,          0.0,            0.0,        1.0);
        
        float xAlpha = orientation.x / 180.0 * M_PI;
        
        mat4 XR = mat4(
                       1.0,   0.0,              0.0,                 0.0,
                       0.0,   cos(xAlpha),      -sin(xAlpha),        0.0,
                       0.0,   sin(xAlpha),      cos(xAlpha),         0.0,
                       0.0,   0.0,              0.0,                 1.0);
        
        mat4 InvXR = mat4(
                          1.0,   0.0,              0.0,                 0.0,
                          0.0,   cos(xAlpha),      sin(xAlpha),        0.0,
                          0.0,   -sin(xAlpha),     cos(xAlpha),         0.0,
                          0.0,   0.0,              0.0,                 1.0);
        
        
        
        mat4 M = S * R * ZR * XR * T;
        mat4 InvM = InvT * InvXR * InvZR * InvR * InvS;
        
        mat4 VP = camera.GetViewMatrix() * camera.GetProjectionMatrix();
        mat4 MVP = M * VP;
        
        s->UploadInvM(InvM);
        s->UploadVP(VP);
        s->UploadMVP(MVP);
        s->UploadM(M);
    }
    
    virtual void Control(float dt) {
    }
    
    virtual void Move(float dt) {
        position = position + velocity * dt;
    }
    
    virtual void Roll(float dt, vec3 direction) {
    }
};

class Avatar : public Object
{
    Shader* shader;
    Mesh *mesh;
    
public:
    Avatar(Mesh *m, vec3 v, vec3 p, vec3 s, vec3 o, vec3 aV) : Object(m, v, p, s, o, aV)
    {
        shader = m->GetShader();
        mesh = m;
        velocity = v;
        position = p;
        scaling = s;
        orientation = o;
        angularV = aV;
    }
    
    vec3 GetOrientation(){
        return vec3(orientation.x,orientation.y,orientation.z);
    }
    
    void FixOrientation() {
        orientation.y += 90;
    }
    
    void Control(float dt) {
        if(keyboardState['j']) {
            if(keyboardState['k']) {orientation.y += dt*45;}
            else {orientation.y -= dt*45;};
            return;
        }
        if(keyboardState['l']) {
            if(keyboardState['k']) {orientation.y -= dt*45;}
            else {orientation.y += dt*45;};
            return;
        }
        if(keyboardState['i']) {
            velocity = vec3(-2*cos((orientation.y-90) / 180.0 * M_PI), 0, -2*sin((orientation.y-90) / 180.0 * M_PI));
            return;
        }
        if(keyboardState['k']) {
            velocity = vec3(2*cos((orientation.y-90) / 180.0 * M_PI), 0, 2*sin((orientation.y-90) / 180.0 * M_PI));
            return;
        }
        if(keyboardState['s']) {
            velocity = vec3(-2*cos((orientation.y-90) / 180.0 * M_PI)* 3.75, 0.75, -2*sin((orientation.y-90) / 180.0 * M_PI))* 3.75;
            return;
        }
        if(keyboardState['d']) {
            position.y = -0.75;
        }
        if(keyboardState['1']) {
            view_toggle = 0;
            position.y = -0.75;
            position.x = 0;
            position.z = 0;
            orientation.y = 180*31;
        }
        if(keyboardState['2']) {
            view_toggle = 4;
            position.y = -0.75;
            position.x = 25;
            position.z = -12;
            orientation.y = 90*61;
        }
        if(keyboardState['9']) {
            orientation.y = 90*63;
            position.y = 5.75;
        }
        velocity = vec3(0.0, 0, 0);
     }
     
    void Move(float dt) {
        position = position + velocity * dt;
    }
    
    void Draw()
    {
        shader->Run();
        
        UploadAttributes(shader);
        
        vec3 eye = camera.GetEyePosition();
        pointlight.SetPointLightSource(eye+vec3(2, 2.5, 2));
        pointlight.UploadAttributes(shader);
        
        camera.UploadAttributes(shader);
        
        mesh->Draw();
    }
    
    virtual void DrawShadow(Shader* shadowShader)
    {
        shadowShader->Run();
        
        UploadAttributes(shadowShader);
        
        vec3 eye = camera.GetEyePosition();
        pointlight.SetPointLightSource(eye+vec3(2, 2.5, 2));
        pointlight.UploadAttributes(shadowShader);
        
        camera.UploadAttributes(shadowShader);
        
        mesh->Draw();
    }

};

class Wheels : public Object
{
    Shader* shader;
    Mesh *mesh;
    int WheelNum;
    float axisOrientation;
    vec3 axisK;
    
public:
    Wheels(Mesh *m, vec3 v, vec3 p, vec3 s, vec3 o, vec3 aV, int w, int aO, vec3 axis) : Object(m, v, p, s, o, aV)
    {
        shader = m->GetShader();
        mesh = m;
        velocity = v;
        position = p;
        scaling = s;
        orientation = o;
        angularV = aV;
        WheelNum = w;
        axisOrientation = aO;
        axisK = axis;
    }
    
    void DrawShadow(Shader* shadowShader)
    {
    }
    
    void SetPosition(vec3 parentPos, vec3 parentOrientation) {
        float xSet = 0.0, zSet = 0.0, hypSign = 0.0;
        if (WheelNum == 1) {
            xSet = 0.33;
            zSet = 0.7;
            hypSign = 1;
        }
        if (WheelNum == 2) {
            xSet = -0.33;
            zSet = 0.7;
            hypSign = 1;
        }
        if (WheelNum == 3) {
            xSet = -0.33;
            zSet = -0.52;
            hypSign = -1;
        }
        if (WheelNum == 4) {
            xSet = 0.33;
            zSet = -0.52;
            hypSign = -1;
        }
        float theta = atan(xSet/zSet) * 180.0 / M_PI;
        float hyp = sqrt((xSet)*(xSet) + (zSet)*(zSet)) * hypSign;
        float xOffest = sin((360 - parentOrientation.y + theta) /180.0 * M_PI) * hyp;
        float yOffest = cos((360 - parentOrientation.y + theta) /180.0 * M_PI) * hyp;
        position = parentPos + vec3(xOffest, -0.15, yOffest);
        orientation = parentOrientation;
    }
    
    void Control(vec3 parentOrientation) {
        if (keyboardState['j'] and WheelNum <= 2) {
            orientation.y = parentOrientation.y - 30;
        }
        if (keyboardState['l'] and WheelNum <= 2) {
            orientation.y = parentOrientation.y + 30;
        }
        float yAngle = 0.0 /180.0 * M_PI;
        float xComp = sin(yAngle);
        float zComp = cos(yAngle);
        axisK = cross(vec3(0, 1, 0), vec3(xComp, 0, zComp));
    }
    
    virtual void Move(float dt) {
        position = position + velocity * dt;
        if (keyboardState['i']) {
            axisOrientation -= 500*dt;
        }
        if (keyboardState['k']) {
            axisOrientation += 500*dt;
        }
        if (keyboardState['s']) {
            axisOrientation -= 800*dt;
        }
    }
    
    void UploadAttributes(Shader *s)
    {
        mat4 T = mat4(
                      1.0,            0.0,            0.0,            0.0,
                      0.0,            1.0,            0.0,            0.0,
                      0.0,            0.0,            1.0,            0.0,
                      position.x,        position.y,        position.z,        1.0);
        
        mat4 InvT = mat4(
                         1.0,            0.0,            0.0,            0.0,
                         0.0,            1.0,            0.0,            0.0,
                         0.0,            0.0,            1.0,            0.0,
                         -position.x,    -position.y,    -position.z,    1.0);
        
        mat4 S = mat4(
                      scaling.x,        0.0,            0.0,            0.0,
                      0.0,            scaling.y,        0.0,            0.0,
                      0.0,            0.0,            scaling.z,        0.0,
                      0.0,            0.0,            0.0,            1.0);
        
        mat4 InvS = mat4(
                         1.0/scaling.x,    0.0,            0.0,            0.0,
                         0.0,            1.0/scaling.y,    0.0,            0.0,
                         0.0,            0.0,            1.0/scaling.z,    0.0,
                         0.0,            0.0,            0.0,            1.0);
        
        float alpha = orientation.y / 180.0 * M_PI;
        
        mat4 R = mat4(
                      cos(alpha),     0.0,            sin(alpha),     0.0,
                      0.0,            1.0,            0.0,            0.0,
                      -sin(alpha),    0.0,            cos(alpha),     0.0,
                      0.0,            0.0,            0.0,            1.0);
        
        mat4 InvR = mat4(
                         cos(alpha),     0.0,            -sin(alpha),    0.0,
                         0.0,            1.0,            0.0,            0.0,
                         sin(alpha),     0.0,            cos(alpha),     0.0,
                         0.0,            0.0,            0.0,            1.0);
        
        float axisTheta = axisOrientation / 180.0 * M_PI;
        
        mat4 K = mat4(
                      0,              -axisK.z,       axisK.y,        0.0,
                      axisK.z,        0.0,            -axisK.x,       0.0,
                      -axisK.y,       axisK.x,        0.0,            0.0,
                      0.0,            0.0,            0.0,            1.0);
        
        mat4 I = mat4(
                      1,        0,       0,        0,
                      0,        1,       0,        0,
                      0,        0,       1,        0,
                      0,        0,       0,        1);
        
        mat4 axisR = I + K*sin(axisTheta) + (K*K)*(1 - cos(axisTheta));
        axisR.m[3][3] = 1;
        mat4 InvAxisR = axisR.transpose();
        
        mat4 M = S * axisR * R * T;
        mat4 InvM = InvT * InvR * InvAxisR * InvS;
        mat4 VP = camera.GetViewMatrix() * camera.GetProjectionMatrix();
        mat4 MVP = M * VP;
        
        s->UploadInvM(InvM);
        s->UploadVP(VP);
        s->UploadMVP(MVP);
        s->UploadM(M);
    }
};

class Scene
{
    InfiniteQuadShader *infiniteQuadShader;
    MeshShader *meshShader;
    ShadowShader *shadowShader;
    MarbleShader *marbleShader;
    Avatar *avatar;
    Wheels *wheel1;
    Wheels *wheel2;
    Wheels *wheel3;
    Wheels *wheel4;
    
    std::vector<Texture*> textures;
    std::vector<Material*> materials;
    std::vector<Geometry*> geometries;
    std::vector<Mesh*> meshes;
    std::vector<Object*> objects;
    
public:
    Scene()
    {
        meshShader = 0;
        infiniteQuadShader = 0;
        shadowShader = 0;
        marbleShader = 0;
        avatar = 0;
        wheel1 = 0;
        wheel2 = 0;
        wheel3 = 0;
        wheel4 = 0;
    }
    
    void Initialize()
    {
        meshShader = new MeshShader();
        infiniteQuadShader = new InfiniteQuadShader();
        shadowShader = new ShadowShader();
        marbleShader = new MarbleShader();
        
        //chevy (all parts)
        textures.push_back(new Texture("./tools/chevy/chevy.png"));
        //tree
        textures.push_back(new Texture("./tools/tree/tree.png"));
        //ball
        textures.push_back(new Texture("./tools/sphere/sphere.png"));
        //floor
        textures.push_back(new Texture("./tools/vasarely_floor.png"));
        
        //chevy
        materials.push_back(new Material(meshShader, vec3(0.0, 0.0, 0.0), vec3(0.5, 0.5, 0.5), vec3(0.5, 0.5, 0.5), 60.0, textures[0]));
        //tree
        materials.push_back(new Material(meshShader, vec3(0.1, 0.1, 0.1), vec3(0.9, 0.9, 0.9), vec3(0.0, 0.0, 0.0), 0.0,  textures[1]));
        //ball
        materials.push_back(new Material(meshShader, vec3(0.1, 0.1, 0.1), vec3(0.6, 0.6, 0.6), vec3(0.3, 0.3, 0.3), 50.0, textures[2]));
        //floor
        materials.push_back(new Material(infiniteQuadShader, vec3(0.2, 0.2, 0.2), vec3(0.7, 0.7, 0.7), vec3(0.2, 0.2, 0.2), 1.0, textures[3]));
        //ball2
        materials.push_back(new Material(marbleShader, vec3(0.1, 0.1, 0.1), vec3(0.5, 0.5, 0.5), vec3(0.1, 0.1, 0.1), 1.0));
        //wheels
        materials.push_back(new Material(meshShader, vec3(0.1, 0.1, 0.1), vec3(0.6, 0.6, 0.6), vec3(0.3, 0.3, 0.3), 100.0, textures[0]));
        
        geometries.push_back(new PolygonalMesh("./tools/chevy/chassis.obj"));
        geometries.push_back(new PolygonalMesh("./tools/chevy/wheel.obj"));
        geometries.push_back(new PolygonalMesh("./tools/tree/tree.obj"));
        geometries.push_back(new PolygonalMesh("./tools/sphere/sphere.obj"));
        geometries.push_back(new InfiniteTexturedQuad());
        
        //chassis
        meshes.push_back(new Mesh(geometries[0], materials[0]));
        //wheel
        meshes.push_back(new Mesh(geometries[1], materials[5]));
        //normal tree
        meshes.push_back(new Mesh(geometries[2], materials[1]));
        //spinning ball
        meshes.push_back(new Mesh(geometries[3], materials[2]));
        //infinite plane
        meshes.push_back(new Mesh(geometries[4], materials[3]));
        //spinning ball2
        meshes.push_back(new Mesh(geometries[3], materials[4]));
        
        //chasis
        avatar = new Avatar(meshes[0], vec3(0,0,0), vec3(0.0, -0.75, 0.0), vec3(0.05, 0.05, 0.05), vec3(0.0, 180.0, 0.0), vec3(0.0, 0.0, 0.0));
        objects.push_back(avatar);
        //4 wheel
        wheel1 = new Wheels(meshes[1], vec3(0,0,0), vec3(0.0, -1.05, -0.5), vec3(0.05, 0.05, 0.05), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0), 1, 0.0, vec3(0,0,0));
        wheel2 = new Wheels(meshes[1], vec3(0,0,0), vec3(0.0, -1.05, -0.5), vec3(0.05, 0.05, 0.05), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0), 2, 0.0, vec3(0,0,0));
        wheel3 = new Wheels(meshes[1], vec3(0,0,0), vec3(0.0, -1.05, -0.5), vec3(0.05, 0.05, 0.05), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0), 3, 0.0, vec3(0,0,0));
        wheel4 = new Wheels(meshes[1], vec3(0,0,0), vec3(0.0, -1.05, -0.5), vec3(0.05, 0.05, 0.05), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0), 4, 0.0, vec3(0,0,0));
        objects.push_back(wheel1);
        objects.push_back(wheel2);
        objects.push_back(wheel3);
        objects.push_back(wheel4);
        
        //normal trees
        for (int j = 1; j <= 6; j++) {
            for (int i = 1; i <= 16; ++i) {
                        objects.push_back(new Object(meshes[2], vec3(0,0,0), vec3(-15 + 5*j, -0.9, 1.5 - 4.5*i), vec3(0.05, 0.05, 0.05), vec3(0, -35*i*j, 0)));
            }
        }
        //infinite plane
        objects.push_back(new Object(meshes[4], vec3(0,0,0), vec3(0.0, -1.0, 0.0), vec3(10.0, 1.0, 10.0)));
    }
    
    ~Scene()
    {
        for(int i = 0; i < textures.size(); i++) delete textures[i];
        for(int i = 0; i < materials.size(); i++) delete materials[i];
        for(int i = 0; i < geometries.size(); i++) delete geometries[i];
        for(int i = 0; i < meshes.size(); i++) delete meshes[i];
        for(int i = 0; i < objects.size(); i++) delete objects[i];
        
        if(meshShader) delete meshShader;
        if(infiniteQuadShader) delete infiniteQuadShader;
        if(shadowShader) delete shadowShader;
    }
    
    void Draw()
    {
        for(int i = 1; i < objects.size() - 1; i++) objects[i]->SpotlightDrawShadow(shadowShader);
        for(int i = 0; i < objects.size() - 1; i++) objects[i]->DrawShadow(shadowShader);
        for(int i = 0; i < objects.size(); i++) objects[i]->Draw();
    }
    
    void Move(float dt) {
        avatar->Control(dt);
        avatar->Move(dt);
        wheel1->SetPosition(avatar->GetPosition(), avatar->GetOrientation());
        wheel2->SetPosition(avatar->GetPosition(), avatar->GetOrientation());
        wheel3->SetPosition(avatar->GetPosition(), avatar->GetOrientation());
        wheel4->SetPosition(avatar->GetPosition(), avatar->GetOrientation());
        wheel1->Control(avatar->GetOrientation());
        wheel2->Control(avatar->GetOrientation());
        wheel3->Control(avatar->GetOrientation());
        wheel4->Control(avatar->GetOrientation());
        wheel1->Move(dt);
        wheel2->Move(dt);
        wheel3->Move(dt);
        wheel4->Move(dt);
    }
    
    vec3 GetAvatarPos()
    {
        return avatar->GetPosition();
    }
    
    vec3 GetAvatarOrientation()
    {
        return avatar->GetOrientation();
    }
    
    bool Between(float mainPoint, float vertex1, float vertex2) {
        if ((mainPoint >= vertex1) and (mainPoint <= vertex2)) {
            return true;
        }
        if ((mainPoint <= vertex1) and (mainPoint >= vertex2)) {
            return true;
        }
        return false;
    }
    
};

vec3 rollDir = vec3(0,0,0);
vec3 rollDir2 = vec3(0,0,0);
Scene scene;

void onInitialization()
{
    glViewport(0, 0, windowWidth, windowHeight);
    
    scene.Initialize();
}

void onExit()
{
    printf("exit");
}

void onDisplay()
{
    vec3 avatP = scene.GetAvatarPos();
    vec3 color1 = vec3(cos(avatP.z/10)*0.5 * cos(avatP.y), 0.0, tan(avatP.x/30)*0.4) + vec3(0.1, 0.2, 0.35) * cos(avatP.x/100);
    vec3 color = color1*0.9 + vec3(0.3, 0.1, 0.2);
    glClearColor(color.x, color.y, color.z, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    scene.Draw();
    
    glutSwapBuffers();
    
}

void onKeyboard(unsigned char key, int x, int y)
{
    keyboardState[key] = true;
    if (key == 'f') {
        view_toggle = 4;
    }
    else if (key == 't') {
        view_toggle += 1;
        view_toggle = view_toggle % 7;
    }
}

void onKeyboardUp(unsigned char key, int x, int y)
{
    keyboardState[key] = false;
}

void onReshape(int winWidth, int winHeight)
{
    camera.SetAspectRatio((float)winWidth / winHeight);
    glViewport(0, 0, winWidth, winHeight);
}

void onIdle( ) {
    double t = glutGet(GLUT_ELAPSED_TIME) * 0.001;
    static double lastTime = 0.0;
    double dt = t - lastTime;
    lastTime = t;
    vec3 AvatarPos = scene.GetAvatarPos();
    vec3 AvatarO = scene.GetAvatarOrientation();
    
    if (view_toggle == 1) {
        camera.SideView(AvatarPos);
    }
    else if (view_toggle == 2) {
        camera.AerialView(AvatarPos, AvatarO.y);
    }
    else if (view_toggle == 3) {
        camera.FrontView(AvatarPos);
    }
    else if (view_toggle == 4) {
        camera.Flyover1(AvatarPos, AvatarO.y);
    }
    else if (view_toggle == 5) {
        camera.Flyover2(AvatarPos, AvatarO.y);
    }
    else if (view_toggle == 6) {
        camera.Flyover3(AvatarPos, AvatarO.y);
    }
    else {
    camera.Helicam(AvatarPos, AvatarO.y);
    }

    scene.Move(dt);
    
    glutPostRedisplay();
}

int main(int argc, char * argv[])
{
    glutInit(&argc, argv);
#if !defined(__APPLE__)
    glutInitContextVersion(majorVersion, minorVersion);
#endif
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(50, 50);
#if defined(__APPLE__)
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutCreateWindow("3D Game");
    
#if !defined(__APPLE__)
    glewExperimental = true;
    glewInit();
#endif
    printf("GL Vendor    : %s\n", glGetString(GL_VENDOR));
    printf("GL Renderer  : %s\n", glGetString(GL_RENDERER));
    printf("GL Version (string)  : %s\n", glGetString(GL_VERSION));
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
    printf("GL Version (integer) : %d.%d\n", majorVersion, minorVersion);
    printf("GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    onInitialization();
    
    glutDisplayFunc(onDisplay);
    glutIdleFunc(onIdle);
    glutKeyboardFunc(onKeyboard);
    glutKeyboardUpFunc(onKeyboardUp);
    glutReshapeFunc(onReshape);
    
    glutMainLoop();
    onExit();
    return 1;
}
