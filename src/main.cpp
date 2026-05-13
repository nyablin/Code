#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef min
#undef max
#endif
#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef GL_VERSION_1_5
using GLsizeiptr=std::ptrdiff_t;
#endif
#ifndef GL_VERSION_2_0
using GLchar=char;
#endif
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif
#ifndef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW 0x88E8
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif
#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH 0x8B84
#endif
constexpr float Pi=3.14159265358979323846f;
constexpr int ChunkSize=16;
constexpr int WorldHeight=48;
constexpr int ChunkRadius=2;
struct Vec3{float x;float y;float z;};
struct Mat4{float m[16];};
struct Vertex{Vec3 p;Vec3 n;Vec3 c;};
struct UiVertex{float x;float y;Vec3 c;};
struct Mesh{GLuint vao=0;GLuint vbo=0;GLuint ebo=0;GLsizei indices=0;};
struct UiMesh{GLuint vao=0;GLuint vbo=0;GLsizei vertices=0;};
struct Chunk{int cx=0;int cz=0;std::vector<unsigned char> blocks;Mesh mesh;bool dirty=true;};
struct RayHit{bool hit=false;int x=0;int y=0;int z=0;int px=0;int py=0;int pz=0;unsigned char type=0;};
struct Camera{Vec3 p{0.0f,22.0f,0.0f};float yaw=-90.0f;float pitch=-18.0f;double lastX=640.0;double lastY=360.0;bool firstMouse=true;};
struct Player{Vec3 p{0.0f,24.0f,0.0f};Vec3 v{0.0f,0.0f,0.0f};bool grounded=false;};
struct App{Camera camera;Player player;bool wireframe=false;bool lastWire=false;bool lastReset=false;bool lastBreak=false;bool lastPlace=false;int selected=0;int inventory[5]{8,8,8,4,6};bool lastSlot[5]{false,false,false,false,false};std::vector<Chunk>* chunks=nullptr;};
using GlCreateShader=GLuint(APIENTRY*)(GLenum);
using GlShaderSource=void(APIENTRY*)(GLuint,GLsizei,const GLchar* const*,const GLint*);
using GlCompileShader=void(APIENTRY*)(GLuint);
using GlGetShaderiv=void(APIENTRY*)(GLuint,GLenum,GLint*);
using GlGetShaderInfoLog=void(APIENTRY*)(GLuint,GLsizei,GLsizei*,GLchar*);
using GlCreateProgram=GLuint(APIENTRY*)();
using GlAttachShader=void(APIENTRY*)(GLuint,GLuint);
using GlLinkProgram=void(APIENTRY*)(GLuint);
using GlGetProgramiv=void(APIENTRY*)(GLuint,GLenum,GLint*);
using GlGetProgramInfoLog=void(APIENTRY*)(GLuint,GLsizei,GLsizei*,GLchar*);
using GlDeleteShader=void(APIENTRY*)(GLuint);
using GlUseProgram=void(APIENTRY*)(GLuint);
using GlGenVertexArrays=void(APIENTRY*)(GLsizei,GLuint*);
using GlDeleteVertexArrays=void(APIENTRY*)(GLsizei,const GLuint*);
using GlBindVertexArray=void(APIENTRY*)(GLuint);
using GlGenBuffers=void(APIENTRY*)(GLsizei,GLuint*);
using GlDeleteBuffers=void(APIENTRY*)(GLsizei,const GLuint*);
using GlBindBuffer=void(APIENTRY*)(GLenum,GLuint);
using GlBufferData=void(APIENTRY*)(GLenum,GLsizeiptr,const void*,GLenum);
using GlEnableVertexAttribArray=void(APIENTRY*)(GLuint);
using GlVertexAttribPointer=void(APIENTRY*)(GLuint,GLint,GLenum,GLboolean,GLsizei,const void*);
using GlGetUniformLocation=GLint(APIENTRY*)(GLuint,const GLchar*);
using GlUniformMatrix4fv=void(APIENTRY*)(GLint,GLsizei,GLboolean,const GLfloat*);
using GlUniform3f=void(APIENTRY*)(GLint,GLfloat,GLfloat,GLfloat);
static GlCreateShader oglCreateShader=nullptr;
static GlShaderSource oglShaderSource=nullptr;
static GlCompileShader oglCompileShader=nullptr;
static GlGetShaderiv oglGetShaderiv=nullptr;
static GlGetShaderInfoLog oglGetShaderInfoLog=nullptr;
static GlCreateProgram oglCreateProgram=nullptr;
static GlAttachShader oglAttachShader=nullptr;
static GlLinkProgram oglLinkProgram=nullptr;
static GlGetProgramiv oglGetProgramiv=nullptr;
static GlGetProgramInfoLog oglGetProgramInfoLog=nullptr;
static GlDeleteShader oglDeleteShader=nullptr;
static GlUseProgram oglUseProgram=nullptr;
static GlGenVertexArrays oglGenVertexArrays=nullptr;
static GlDeleteVertexArrays oglDeleteVertexArrays=nullptr;
static GlBindVertexArray oglBindVertexArray=nullptr;
static GlGenBuffers oglGenBuffers=nullptr;
static GlDeleteBuffers oglDeleteBuffers=nullptr;
static GlBindBuffer oglBindBuffer=nullptr;
static GlBufferData oglBufferData=nullptr;
static GlEnableVertexAttribArray oglEnableVertexAttribArray=nullptr;
static GlVertexAttribPointer oglVertexAttribPointer=nullptr;
static GlGetUniformLocation oglGetUniformLocation=nullptr;
static GlUniformMatrix4fv oglUniformMatrix4fv=nullptr;
static GlUniform3f oglUniform3f=nullptr;
static Vec3 operator+(Vec3 a,Vec3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
static Vec3 operator-(Vec3 a,Vec3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
static Vec3 operator*(Vec3 a,float s){return {a.x*s,a.y*s,a.z*s};}
static Vec3 operator/(Vec3 a,float s){return {a.x/s,a.y/s,a.z/s};}
static Vec3& operator+=(Vec3& a,Vec3 b){a=a+b;return a;}
static float dot(Vec3 a,Vec3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
static Vec3 cross(Vec3 a,Vec3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
static float len(Vec3 v){return std::sqrt(dot(v,v));}
static Vec3 norm(Vec3 v){float l=len(v);return l>0.00001f?v/l:Vec3{0.0f,1.0f,0.0f};}
static Vec3 mix(Vec3 a,Vec3 b,float t){return a*(1.0f-t)+b*t;}
static float fract(float v){return v-std::floor(v);}
static float smooth(float t){return t*t*(3.0f-2.0f*t);}
static float rand2(int x,int z){return fract(std::sin(static_cast<float>(x)*127.1f+static_cast<float>(z)*311.7f)*43758.5453f)*2.0f-1.0f;}
static float noise(float x,float z){int xi=static_cast<int>(std::floor(x));int zi=static_cast<int>(std::floor(z));float xf=x-static_cast<float>(xi);float zf=z-static_cast<float>(zi);float u=smooth(xf);float v=smooth(zf);float a=rand2(xi,zi);float b=rand2(xi+1,zi);float c=rand2(xi,zi+1);float d=rand2(xi+1,zi+1);return (a*(1.0f-u)+b*u)*(1.0f-v)+(c*(1.0f-u)+d*u)*v;}
static Vec3 itemColor(int type){Vec3 colors[5]{{0.30f,0.58f,0.20f},{0.45f,0.31f,0.18f},{0.43f,0.43f,0.42f},{0.86f,0.88f,0.84f},{0.42f,0.70f,0.28f}};return colors[std::clamp(type,0,4)];}
static Vec3 blockColor(unsigned char type,int y){Vec3 c=itemColor(static_cast<int>(std::clamp<unsigned char>(type,1,5))-1);float shade=0.82f+fract(static_cast<float>(y)*0.137f)*0.18f;return c*shade;}
static Mat4 identity(){Mat4 r{};r.m[0]=1.0f;r.m[5]=1.0f;r.m[10]=1.0f;r.m[15]=1.0f;return r;}
static Mat4 mul(Mat4 a,Mat4 b){Mat4 r{};for(int c=0;c<4;c++){for(int row=0;row<4;row++){r.m[c*4+row]=a.m[0*4+row]*b.m[c*4+0]+a.m[1*4+row]*b.m[c*4+1]+a.m[2*4+row]*b.m[c*4+2]+a.m[3*4+row]*b.m[c*4+3];}}return r;}
static Mat4 perspective(float fov,float aspect,float nearPlane,float farPlane){Mat4 r{};float t=std::tan(fov*0.5f);r.m[0]=1.0f/(aspect*t);r.m[5]=1.0f/t;r.m[10]=-(farPlane+nearPlane)/(farPlane-nearPlane);r.m[11]=-1.0f;r.m[14]=-(2.0f*farPlane*nearPlane)/(farPlane-nearPlane);return r;}
static Mat4 lookAt(Vec3 eye,Vec3 center,Vec3 up){Vec3 f=norm(center-eye);Vec3 s=norm(cross(f,up));Vec3 u=cross(s,f);Mat4 r=identity();r.m[0]=s.x;r.m[4]=s.y;r.m[8]=s.z;r.m[1]=u.x;r.m[5]=u.y;r.m[9]=u.z;r.m[2]=-f.x;r.m[6]=-f.y;r.m[10]=-f.z;r.m[12]=-dot(s,eye);r.m[13]=-dot(u,eye);r.m[14]=dot(f,eye);return r;}
static Vec3 cameraForward(const Camera& c){float yaw=c.yaw*Pi/180.0f;float pitch=c.pitch*Pi/180.0f;return norm({std::cos(yaw)*std::cos(pitch),std::sin(pitch),std::sin(yaw)*std::cos(pitch)});}
template<class T>static T loadProc(const char* name){auto proc=glfwGetProcAddress(name);if(!proc)throw std::runtime_error(std::string("OpenGL function not found: ")+name);return reinterpret_cast<T>(proc);}
static void loadGl(){oglCreateShader=loadProc<GlCreateShader>("glCreateShader");oglShaderSource=loadProc<GlShaderSource>("glShaderSource");oglCompileShader=loadProc<GlCompileShader>("glCompileShader");oglGetShaderiv=loadProc<GlGetShaderiv>("glGetShaderiv");oglGetShaderInfoLog=loadProc<GlGetShaderInfoLog>("glGetShaderInfoLog");oglCreateProgram=loadProc<GlCreateProgram>("glCreateProgram");oglAttachShader=loadProc<GlAttachShader>("glAttachShader");oglLinkProgram=loadProc<GlLinkProgram>("glLinkProgram");oglGetProgramiv=loadProc<GlGetProgramiv>("glGetProgramiv");oglGetProgramInfoLog=loadProc<GlGetProgramInfoLog>("glGetProgramInfoLog");oglDeleteShader=loadProc<GlDeleteShader>("glDeleteShader");oglUseProgram=loadProc<GlUseProgram>("glUseProgram");oglGenVertexArrays=loadProc<GlGenVertexArrays>("glGenVertexArrays");oglDeleteVertexArrays=loadProc<GlDeleteVertexArrays>("glDeleteVertexArrays");oglBindVertexArray=loadProc<GlBindVertexArray>("glBindVertexArray");oglGenBuffers=loadProc<GlGenBuffers>("glGenBuffers");oglDeleteBuffers=loadProc<GlDeleteBuffers>("glDeleteBuffers");oglBindBuffer=loadProc<GlBindBuffer>("glBindBuffer");oglBufferData=loadProc<GlBufferData>("glBufferData");oglEnableVertexAttribArray=loadProc<GlEnableVertexAttribArray>("glEnableVertexAttribArray");oglVertexAttribPointer=loadProc<GlVertexAttribPointer>("glVertexAttribPointer");oglGetUniformLocation=loadProc<GlGetUniformLocation>("glGetUniformLocation");oglUniformMatrix4fv=loadProc<GlUniformMatrix4fv>("glUniformMatrix4fv");oglUniform3f=loadProc<GlUniform3f>("glUniform3f");}
static GLuint compileShader(GLenum type,const char* source){GLuint shader=oglCreateShader(type);oglShaderSource(shader,1,&source,nullptr);oglCompileShader(shader);GLint ok=0;oglGetShaderiv(shader,GL_COMPILE_STATUS,&ok);if(!ok){GLint size=0;oglGetShaderiv(shader,GL_INFO_LOG_LENGTH,&size);std::string log(static_cast<size_t>(std::max(size,1)),'\0');oglGetShaderInfoLog(shader,size,nullptr,log.data());throw std::runtime_error(log);}return shader;}
static GLuint createProgram(const char* vs,const char* fs){GLuint v=compileShader(GL_VERTEX_SHADER,vs);GLuint f=compileShader(GL_FRAGMENT_SHADER,fs);GLuint p=oglCreateProgram();oglAttachShader(p,v);oglAttachShader(p,f);oglLinkProgram(p);GLint ok=0;oglGetProgramiv(p,GL_LINK_STATUS,&ok);if(!ok){GLint size=0;oglGetProgramiv(p,GL_INFO_LOG_LENGTH,&size);std::string log(static_cast<size_t>(std::max(size,1)),'\0');oglGetProgramInfoLog(p,size,nullptr,log.data());throw std::runtime_error(log);}oglDeleteShader(v);oglDeleteShader(f);return p;}
static void destroyMesh(Mesh& mesh){if(mesh.ebo)oglDeleteBuffers(1,&mesh.ebo);if(mesh.vbo)oglDeleteBuffers(1,&mesh.vbo);if(mesh.vao)oglDeleteVertexArrays(1,&mesh.vao);mesh={};}
static Mesh uploadMesh(const std::vector<Vertex>& vertices,const std::vector<unsigned int>& indices){Mesh mesh;mesh.indices=static_cast<GLsizei>(indices.size());oglGenVertexArrays(1,&mesh.vao);oglGenBuffers(1,&mesh.vbo);oglGenBuffers(1,&mesh.ebo);oglBindVertexArray(mesh.vao);oglBindBuffer(GL_ARRAY_BUFFER,mesh.vbo);oglBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size()*sizeof(Vertex)),vertices.empty()?nullptr:vertices.data(),GL_STATIC_DRAW);oglBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh.ebo);oglBufferData(GL_ELEMENT_ARRAY_BUFFER,static_cast<GLsizeiptr>(indices.size()*sizeof(unsigned int)),indices.empty()?nullptr:indices.data(),GL_STATIC_DRAW);oglEnableVertexAttribArray(0);oglVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,p)));oglEnableVertexAttribArray(1);oglVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,n)));oglEnableVertexAttribArray(2);oglVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,c)));oglBindVertexArray(0);return mesh;}
static UiMesh createUiMesh(){UiMesh mesh;oglGenVertexArrays(1,&mesh.vao);oglGenBuffers(1,&mesh.vbo);oglBindVertexArray(mesh.vao);oglBindBuffer(GL_ARRAY_BUFFER,mesh.vbo);oglEnableVertexAttribArray(0);oglVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(UiVertex),reinterpret_cast<void*>(offsetof(UiVertex,x)));oglEnableVertexAttribArray(1);oglVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(UiVertex),reinterpret_cast<void*>(offsetof(UiVertex,c)));oglBindVertexArray(0);return mesh;}
static Mesh createScreenQuad(){std::vector<Vertex> vertices{{{-1.0f,-1.0f,0.0f},{0.0f,0.0f,1.0f},{1.0f,1.0f,1.0f}},{{1.0f,-1.0f,0.0f},{0.0f,0.0f,1.0f},{1.0f,1.0f,1.0f}},{{-1.0f,1.0f,0.0f},{0.0f,0.0f,1.0f},{1.0f,1.0f,1.0f}},{{1.0f,1.0f,0.0f},{0.0f,0.0f,1.0f},{1.0f,1.0f,1.0f}}};std::vector<unsigned int> indices{0,1,2,2,1,3};return uploadMesh(vertices,indices);}
static int divFloor(int a,int b){int q=a/b;int r=a%b;return r&&((r<0)!=(b<0))?q-1:q;}
static int modFloor(int a,int b){int r=a%b;return r<0?r+b:r;}
static int blockIndex(int x,int y,int z){return (y*ChunkSize+z)*ChunkSize+x;}
static int blockHeight(int x,int z){float h=15.0f+noise(static_cast<float>(x)*0.045f,static_cast<float>(z)*0.045f)*8.0f+noise(static_cast<float>(x)*0.013f+40.0f,static_cast<float>(z)*0.013f)*10.0f;return std::clamp(static_cast<int>(std::floor(h)),4,WorldHeight-8);}
static Chunk* findChunk(std::vector<Chunk>& chunks,int cx,int cz){for(Chunk& c:chunks)if(c.cx==cx&&c.cz==cz)return &c;return nullptr;}
static const Chunk* findChunkConst(const std::vector<Chunk>& chunks,int cx,int cz){for(const Chunk& c:chunks)if(c.cx==cx&&c.cz==cz)return &c;return nullptr;}
static unsigned char getBlock(const std::vector<Chunk>& chunks,int wx,int y,int wz){if(y<0||y>=WorldHeight)return 0;int cx=divFloor(wx,ChunkSize);int cz=divFloor(wz,ChunkSize);int lx=modFloor(wx,ChunkSize);int lz=modFloor(wz,ChunkSize);const Chunk* c=findChunkConst(chunks,cx,cz);return c?c->blocks[blockIndex(lx,y,lz)]:0;}
static void setBlock(std::vector<Chunk>& chunks,int wx,int y,int wz,unsigned char type){if(y<0||y>=WorldHeight)return;int cx=divFloor(wx,ChunkSize);int cz=divFloor(wz,ChunkSize);int lx=modFloor(wx,ChunkSize);int lz=modFloor(wz,ChunkSize);Chunk* c=findChunk(chunks,cx,cz);if(c){c->blocks[blockIndex(lx,y,lz)]=type;c->dirty=true;}}
static void markAllDirty(std::vector<Chunk>& chunks){for(Chunk& c:chunks)c.dirty=true;}
static Chunk makeChunk(int cx,int cz){Chunk c;c.cx=cx;c.cz=cz;c.blocks.assign(static_cast<size_t>(ChunkSize*WorldHeight*ChunkSize),0);for(int z=0;z<ChunkSize;z++){for(int x=0;x<ChunkSize;x++){int wx=cx*ChunkSize+x;int wz=cz*ChunkSize+z;int h=blockHeight(wx,wz);for(int y=0;y<=h;y++){unsigned char type=3;if(y==h)type=h>27?4:1;else if(y>h-4)type=2;if(y<9&&rand2(wx*17+y,wz*19)>0.82f)type=5;c.blocks[blockIndex(x,y,z)]=type;}if(h<WorldHeight-7&&rand2(wx*31,wz*37)>0.92f){for(int y=h+1;y<h+4;y++)c.blocks[blockIndex(x,y,z)]=5;for(int dz=-2;dz<=2;dz++){for(int dx=-2;dx<=2;dx++){for(int dy=3;dy<=5;dy++){int lx=x+dx;int lz=z+dz;int ly=h+dy;if(lx>=0&&lx<ChunkSize&&lz>=0&&lz<ChunkSize&&ly>=0&&ly<WorldHeight&&std::abs(dx)+std::abs(dz)+dy<8)c.blocks[blockIndex(lx,ly,lz)]=5;}}}}}}c.dirty=true;return c;}
static void addFace(std::vector<Vertex>& vertices,std::vector<unsigned int>& indices,float x,float y,float z,int face,Vec3 color){static const Vec3 normals[6]{{1.0f,0.0f,0.0f},{-1.0f,0.0f,0.0f},{0.0f,1.0f,0.0f},{0.0f,-1.0f,0.0f},{0.0f,0.0f,1.0f},{0.0f,0.0f,-1.0f}};static const Vec3 p[6][4]{{{1,0,0},{1,1,0},{1,1,1},{1,0,1}},{{0,0,1},{0,1,1},{0,1,0},{0,0,0}},{{0,1,1},{1,1,1},{1,1,0},{0,1,0}},{{0,0,0},{1,0,0},{1,0,1},{0,0,1}},{{1,0,1},{1,1,1},{0,1,1},{0,0,1}},{{0,0,0},{0,1,0},{1,1,0},{1,0,0}}};unsigned int base=static_cast<unsigned int>(vertices.size());float light[6]{0.82f,0.68f,1.0f,0.50f,0.76f,0.62f};Vec3 c=color*light[face];for(int i=0;i<4;i++)vertices.push_back({{x+p[face][i].x,y+p[face][i].y,z+p[face][i].z},normals[face],c});indices.push_back(base);indices.push_back(base+1);indices.push_back(base+2);indices.push_back(base);indices.push_back(base+2);indices.push_back(base+3);}
static void rebuildChunkMesh(std::vector<Chunk>& chunks,Chunk& chunk){std::vector<Vertex> vertices;std::vector<unsigned int> indices;vertices.reserve(ChunkSize*ChunkSize*256);int dirs[6][3]{{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};for(int y=0;y<WorldHeight;y++){for(int z=0;z<ChunkSize;z++){for(int x=0;x<ChunkSize;x++){unsigned char type=chunk.blocks[blockIndex(x,y,z)];if(!type)continue;int wx=chunk.cx*ChunkSize+x;int wz=chunk.cz*ChunkSize+z;for(int f=0;f<6;f++){if(!getBlock(chunks,wx+dirs[f][0],y+dirs[f][1],wz+dirs[f][2]))addFace(vertices,indices,static_cast<float>(wx),static_cast<float>(y),static_cast<float>(wz),f,blockColor(type,y));}}}}destroyMesh(chunk.mesh);chunk.mesh=uploadMesh(vertices,indices);chunk.dirty=false;}
static void rebuildDirtyChunks(std::vector<Chunk>& chunks){for(size_t i=0;i<chunks.size();i++)if(chunks[i].dirty)rebuildChunkMesh(chunks,chunks[i]);}
static void clearChunks(std::vector<Chunk>& chunks){for(Chunk& c:chunks)destroyMesh(c.mesh);chunks.clear();}
static void ensureChunks(std::vector<Chunk>& chunks,Vec3 p){int bx=static_cast<int>(std::floor(p.x));int bz=static_cast<int>(std::floor(p.z));int pcx=divFloor(bx,ChunkSize);int pcz=divFloor(bz,ChunkSize);bool changed=false;for(size_t i=0;i<chunks.size();){if(std::abs(chunks[i].cx-pcx)>ChunkRadius+1||std::abs(chunks[i].cz-pcz)>ChunkRadius+1){destroyMesh(chunks[i].mesh);chunks.erase(chunks.begin()+static_cast<std::ptrdiff_t>(i));changed=true;}else i++;}for(int z=pcz-ChunkRadius;z<=pcz+ChunkRadius;z++){for(int x=pcx-ChunkRadius;x<=pcx+ChunkRadius;x++){if(!findChunk(chunks,x,z)){chunks.push_back(makeChunk(x,z));changed=true;}}}if(changed)markAllDirty(chunks);}
static int surfaceY(const std::vector<Chunk>& chunks,int x,int z){for(int y=WorldHeight-1;y>=0;y--)if(getBlock(chunks,x,y,z))return y;return blockHeight(x,z);}
static bool playerHits(const std::vector<Chunk>& chunks,Vec3 p){float hw=0.32f;float h=1.78f;int minX=static_cast<int>(std::floor(p.x-hw));int maxX=static_cast<int>(std::floor(p.x+hw));int minY=static_cast<int>(std::floor(p.y+0.03f));int maxY=static_cast<int>(std::floor(p.y+h-0.03f));int minZ=static_cast<int>(std::floor(p.z-hw));int maxZ=static_cast<int>(std::floor(p.z+hw));for(int y=minY;y<=maxY;y++)for(int z=minZ;z<=maxZ;z++)for(int x=minX;x<=maxX;x++)if(getBlock(chunks,x,y,z))return true;return false;}
static bool blockTouchesPlayer(Vec3 p,int x,int y,int z){float hw=0.38f;float h=1.86f;return p.x+hw>static_cast<float>(x)&&p.x-hw<static_cast<float>(x+1)&&p.y+h>static_cast<float>(y)&&p.y<static_cast<float>(y+1)&&p.z+hw>static_cast<float>(z)&&p.z-hw<static_cast<float>(z+1);}
static RayHit raycast(const std::vector<Chunk>& chunks,Vec3 origin,Vec3 dir){RayHit hit;int lastX=static_cast<int>(std::floor(origin.x));int lastY=static_cast<int>(std::floor(origin.y));int lastZ=static_cast<int>(std::floor(origin.z));for(float t=0.0f;t<6.5f;t+=0.05f){Vec3 p=origin+dir*t;int x=static_cast<int>(std::floor(p.x));int y=static_cast<int>(std::floor(p.y));int z=static_cast<int>(std::floor(p.z));unsigned char type=getBlock(chunks,x,y,z);if(type){hit.hit=true;hit.x=x;hit.y=y;hit.z=z;hit.px=lastX;hit.py=lastY;hit.pz=lastZ;hit.type=type;return hit;}lastX=x;lastY=y;lastZ=z;}return hit;}
static void cursorCallback(GLFWwindow* window,double xpos,double ypos){auto* app=static_cast<App*>(glfwGetWindowUserPointer(window));if(!app)return;Camera& c=app->camera;if(c.firstMouse){c.lastX=xpos;c.lastY=ypos;c.firstMouse=false;}float dx=static_cast<float>(xpos-c.lastX);float dy=static_cast<float>(c.lastY-ypos);c.lastX=xpos;c.lastY=ypos;float sens=0.09f;c.yaw+=dx*sens;c.pitch=std::clamp(c.pitch+dy*sens,-84.0f,84.0f);}
static bool pressed(GLFWwindow* window,int key){return glfwGetKey(window,key)==GLFW_PRESS;}
static void moveAxis(App& app,float dx,float dy,float dz,bool step){if(!app.chunks)return;Vec3 old=app.player.p;Vec3 next{old.x+dx,old.y+dy,old.z+dz};if(!playerHits(*app.chunks,next)){app.player.p=next;return;}if(step&&(dx!=0.0f||dz!=0.0f)){Vec3 up{old.x+dx,old.y+1.02f,old.z+dz};if(!playerHits(*app.chunks,up)){app.player.p=up;return;}}if(dx!=0.0f)app.player.v.x=0.0f;if(dy!=0.0f){if(dy<0.0f)app.player.grounded=true;app.player.v.y=0.0f;}if(dz!=0.0f)app.player.v.z=0.0f;}
static void updateInput(GLFWwindow* window,App& app,float dt){if(!app.chunks)return;if(pressed(window,GLFW_KEY_ESCAPE))glfwSetWindowShouldClose(window,GLFW_TRUE);bool wireNow=pressed(window,GLFW_KEY_F);if(wireNow&&!app.lastWire)app.wireframe=!app.wireframe;app.lastWire=wireNow;bool resetNow=pressed(window,GLFW_KEY_R);if(resetNow&&!app.lastReset){clearChunks(*app.chunks);for(int& n:app.inventory)n=0;app.inventory[0]=8;app.inventory[1]=8;app.inventory[2]=8;app.inventory[3]=4;app.inventory[4]=6;app.player.p={0.0f,32.0f,0.0f};app.player.v={0.0f,0.0f,0.0f};}app.lastReset=resetNow;int keys[5]{GLFW_KEY_1,GLFW_KEY_2,GLFW_KEY_3,GLFW_KEY_4,GLFW_KEY_5};for(int i=0;i<5;i++){bool down=pressed(window,keys[i]);if(down&&!app.lastSlot[i])app.selected=i;app.lastSlot[i]=down;}Vec3 eye=app.player.p+Vec3{0.0f,1.62f,0.0f};Vec3 dir=cameraForward(app.camera);bool breakNow=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;if(breakNow&&!app.lastBreak){RayHit hit=raycast(*app.chunks,eye,dir);if(hit.hit&&hit.y>0){setBlock(*app.chunks,hit.x,hit.y,hit.z,0);app.inventory[std::clamp(static_cast<int>(hit.type)-1,0,4)]++;markAllDirty(*app.chunks);}}app.lastBreak=breakNow;bool placeNow=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS;if(placeNow&&!app.lastPlace){RayHit hit=raycast(*app.chunks,eye,dir);if(hit.hit&&app.inventory[app.selected]>0&&hit.py>=0&&hit.py<WorldHeight&&!getBlock(*app.chunks,hit.px,hit.py,hit.pz)&&!blockTouchesPlayer(app.player.p,hit.px,hit.py,hit.pz)){setBlock(*app.chunks,hit.px,hit.py,hit.pz,static_cast<unsigned char>(app.selected+1));app.inventory[app.selected]--;markAllDirty(*app.chunks);}}app.lastPlace=placeNow;Vec3 f=dir;Vec3 flat=norm({f.x,0.0f,f.z});Vec3 right=norm(cross(flat,{0.0f,1.0f,0.0f}));Vec3 wish{0.0f,0.0f,0.0f};if(pressed(window,GLFW_KEY_W))wish+=flat;if(pressed(window,GLFW_KEY_S))wish+=flat*-1.0f;if(pressed(window,GLFW_KEY_D))wish+=right;if(pressed(window,GLFW_KEY_A))wish+=right*-1.0f;if(len(wish)>0.01f)wish=norm(wish);float speed=pressed(window,GLFW_KEY_LEFT_SHIFT)||pressed(window,GLFW_KEY_RIGHT_SHIFT)?7.2f:4.8f;app.player.v.x=wish.x*speed;app.player.v.z=wish.z*speed;if(pressed(window,GLFW_KEY_SPACE)&&app.player.grounded){app.player.v.y=8.2f;app.player.grounded=false;}app.player.v.y-=22.0f*dt;bool canStep=app.player.grounded;app.player.grounded=false;moveAxis(app,app.player.v.x*dt,0.0f,0.0f,canStep);moveAxis(app,0.0f,0.0f,app.player.v.z*dt,canStep);moveAxis(app,0.0f,app.player.v.y*dt,0.0f,false);if(app.player.p.y<-16.0f){int sy=surfaceY(*app.chunks,0,0)+2;app.player.p={0.5f,static_cast<float>(sy),0.5f};app.player.v={0.0f,0.0f,0.0f};}app.camera.p=app.player.p+Vec3{0.0f,1.62f,0.0f};}
static void pushRect(std::vector<UiVertex>& v,float x0,float y0,float x1,float y1,Vec3 c){v.push_back({x0,y0,c});v.push_back({x1,y0,c});v.push_back({x0,y1,c});v.push_back({x0,y1,c});v.push_back({x1,y0,c});v.push_back({x1,y1,c});}
static void uploadUi(UiMesh& mesh,const std::vector<UiVertex>& vertices){mesh.vertices=static_cast<GLsizei>(vertices.size());oglBindVertexArray(mesh.vao);oglBindBuffer(GL_ARRAY_BUFFER,mesh.vbo);oglBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size()*sizeof(UiVertex)),vertices.empty()?nullptr:vertices.data(),GL_DYNAMIC_DRAW);}
static void drawHotbar(UiMesh& mesh,GLuint program,const App& app,int w,int h){std::vector<UiVertex> vertices;float sy=0.15f;float sx=sy*static_cast<float>(std::max(h,1))/static_cast<float>(std::max(w,1));float gap=sx*0.18f;float total=sx*5.0f+gap*4.0f;float y0=-0.92f;float padY=sy*0.10f;float padX=padY*static_cast<float>(std::max(h,1))/static_cast<float>(std::max(w,1));float borderY=sy*0.035f;float borderX=borderY*static_cast<float>(std::max(h,1))/static_cast<float>(std::max(w,1));float cx=0.0f;float cy=0.0f;float cw=0.012f;float ch=cw*static_cast<float>(std::max(w,1))/static_cast<float>(std::max(h,1));pushRect(vertices,cx-cw,cy-ch*0.15f,cx+cw,cy+ch*0.15f,{0.92f,0.92f,0.86f});pushRect(vertices,cx-cw*0.15f,cy-ch,cx+cw*0.15f,cy+ch,{0.92f,0.92f,0.86f});for(int i=0;i<5;i++){float x0=-total*0.5f+static_cast<float>(i)*(sx+gap);float x1=x0+sx;float y1=y0+sy;Vec3 bg=i==app.selected?Vec3{0.24f,0.25f,0.27f}:Vec3{0.11f,0.12f,0.14f};pushRect(vertices,x0,y0,x1,y1,bg);if(i==app.selected){Vec3 edge{0.95f,0.86f,0.42f};pushRect(vertices,x0-borderX,y0-borderY,x1+borderX,y0,edge);pushRect(vertices,x0-borderX,y1,x1+borderX,y1+borderY,edge);pushRect(vertices,x0-borderX,y0,x0,y1,edge);pushRect(vertices,x1,y0,x1+borderX,y1,edge);}Vec3 color=app.inventory[i]>0?itemColor(i):mix(itemColor(i),{0.04f,0.05f,0.06f},0.72f);pushRect(vertices,x0+padX,y0+padY,x1-padX,y1-padY,color);int bars=std::min(app.inventory[i],10);for(int b=0;b<bars;b++){float by0=y0+padY+static_cast<float>(b)*(sy-padY*2.0f)/10.0f;float by1=by0+(sy-padY*2.0f)/13.0f;pushRect(vertices,x1-padX*1.7f,by0,x1-padX*0.7f,by1,{0.96f,0.96f,0.88f});}}uploadUi(mesh,vertices);glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);oglUseProgram(program);oglBindVertexArray(mesh.vao);glDrawArrays(GL_TRIANGLES,0,mesh.vertices);}
static void drawMesh(const Mesh& mesh,GLint uModel,GLint uMvp,GLint uTint,Mat4 projection,Mat4 view,Mat4 modelMatrix,Vec3 tint){if(!mesh.indices)return;Mat4 mvp=mul(projection,mul(view,modelMatrix));oglUniformMatrix4fv(uModel,1,GL_FALSE,modelMatrix.m);oglUniformMatrix4fv(uMvp,1,GL_FALSE,mvp.m);oglUniform3f(uTint,tint.x,tint.y,tint.z);oglBindVertexArray(mesh.vao);glDrawElements(GL_TRIANGLES,mesh.indices,GL_UNSIGNED_INT,nullptr);}
static const char* worldVs=R"glsl(#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec3 aColor;
uniform mat4 uModel;
uniform mat4 uMvp;
out vec3 vNormal;
out vec3 vColor;
out vec3 vPos;
void main(){
vec4 world=uModel*vec4(aPos,1.0);
vPos=world.xyz;
vNormal=normalize(mat3(uModel)*aNormal);
vColor=aColor;
gl_Position=uMvp*vec4(aPos,1.0);
})glsl";
static const char* worldFs=R"glsl(#version 330 core
in vec3 vNormal;
in vec3 vColor;
in vec3 vPos;
uniform vec3 uTint;
uniform vec3 uSunDir;
uniform vec3 uSunColor;
uniform vec3 uSkyColor;
uniform vec3 uCamera;
out vec4 FragColor;
void main(){
vec3 n=normalize(vNormal);
vec3 viewDir=normalize(uCamera-vPos);
float diffuse=max(dot(n,normalize(uSunDir)),0.0);
float wrap=diffuse*0.86+0.28;
float spec=pow(max(dot(reflect(-normalize(uSunDir),n),viewDir),0.0),42.0)*0.18;
float fog=clamp(length(vPos-uCamera)/118.0,0.0,1.0);
vec3 color=vColor*uTint*wrap+uSunColor*spec;
FragColor=vec4(mix(color,uSkyColor,fog),1.0);
})glsl";
static const char* skyVs=R"glsl(#version 330 core
layout(location=0) in vec3 aPos;
uniform vec3 uForward;
uniform vec3 uRight;
uniform vec3 uUp;
out vec3 vDir;
void main(){
vDir=normalize(uForward+uRight*aPos.x+uUp*aPos.y);
gl_Position=vec4(aPos.xy,0.0,1.0);
})glsl";
static const char* skyFs=R"glsl(#version 330 core
in vec3 vDir;
uniform vec3 uSunDir;
out vec4 FragColor;
void main(){
vec3 dir=normalize(vDir);
float h=clamp(dir.y*0.5+0.5,0.0,1.0);
vec3 horizon=vec3(0.78,0.84,0.92);
vec3 zenith=vec3(0.18,0.42,0.82);
float sun=max(dot(dir,normalize(uSunDir)),0.0);
float disk=smoothstep(0.997,0.9995,sun);
float glow=pow(sun,48.0)*0.35+pow(sun,512.0)*0.7;
vec3 color=mix(horizon,zenith,h)+vec3(1.0,0.64,0.24)*glow+vec3(1.0,0.88,0.55)*disk;
FragColor=vec4(color,1.0);
})glsl";
static const char* uiVs=R"glsl(#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec3 aColor;
out vec3 vColor;
void main(){
vColor=aColor;
gl_Position=vec4(aPos,0.0,1.0);
})glsl";
static const char* uiFs=R"glsl(#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main(){
FragColor=vec4(vColor,1.0);
})glsl";
int main(int argc,char** argv){try{bool smoke=argc>1&&std::string(argv[1])=="--smoke";if(!glfwInit())throw std::runtime_error("GLFW init failed");glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);if(smoke)glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
#if defined(__APPLE__)
glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GLFW_TRUE);
#endif
GLFWwindow* window=glfwCreateWindow(1280,720,"OpenGLWorld",nullptr,nullptr);if(!window)throw std::runtime_error("Window creation failed");glfwMakeContextCurrent(window);glfwSwapInterval(1);loadGl();App app;std::vector<Chunk> chunks;app.chunks=&chunks;ensureChunks(chunks,app.player.p);int sy=surfaceY(chunks,0,0)+2;app.player.p={0.5f,static_cast<float>(sy),0.5f};app.camera.p=app.player.p+Vec3{0.0f,1.62f,0.0f};rebuildDirtyChunks(chunks);glfwSetWindowUserPointer(window,&app);glfwSetCursorPosCallback(window,cursorCallback);glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);GLuint worldProgram=createProgram(worldVs,worldFs);GLuint skyProgram=createProgram(skyVs,skyFs);GLuint uiProgram=createProgram(uiVs,uiFs);GLint uModel=oglGetUniformLocation(worldProgram,"uModel");GLint uMvp=oglGetUniformLocation(worldProgram,"uMvp");GLint uTint=oglGetUniformLocation(worldProgram,"uTint");GLint uSunDir=oglGetUniformLocation(worldProgram,"uSunDir");GLint uSunColor=oglGetUniformLocation(worldProgram,"uSunColor");GLint uSkyColor=oglGetUniformLocation(worldProgram,"uSkyColor");GLint uCamera=oglGetUniformLocation(worldProgram,"uCamera");GLint sForward=oglGetUniformLocation(skyProgram,"uForward");GLint sRight=oglGetUniformLocation(skyProgram,"uRight");GLint sUp=oglGetUniformLocation(skyProgram,"uUp");GLint sSunDir=oglGetUniformLocation(skyProgram,"uSunDir");Mesh skyQuad=createScreenQuad();UiMesh uiMesh=createUiMesh();if(smoke){clearChunks(chunks);glfwTerminate();return 0;}Vec3 sunDir=norm({-0.38f,0.78f,-0.48f});Vec3 sunColor{1.0f,0.88f,0.62f};Vec3 skyColor{0.62f,0.76f,0.92f};double last=glfwGetTime();while(!glfwWindowShouldClose(window)){double now=glfwGetTime();float dt=std::min(static_cast<float>(now-last),0.033f);last=now;ensureChunks(chunks,app.player.p);glfwPollEvents();updateInput(window,app,dt);ensureChunks(chunks,app.player.p);rebuildDirtyChunks(chunks);int w=0;int h=0;glfwGetFramebufferSize(window,&w,&h);float aspect=static_cast<float>(std::max(w,1))/static_cast<float>(std::max(h,1));Vec3 forward=cameraForward(app.camera);Vec3 right=norm(cross(forward,{0.0f,1.0f,0.0f}));Vec3 up=norm(cross(right,forward));Mat4 proj=perspective(65.0f*Pi/180.0f,aspect,0.1f,320.0f);Mat4 view=lookAt(app.camera.p,app.camera.p+forward,{0.0f,1.0f,0.0f});glViewport(0,0,w,h);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);oglUseProgram(skyProgram);oglUniform3f(sForward,forward.x,forward.y,forward.z);oglUniform3f(sRight,right.x*aspect*0.72f,right.y*aspect*0.72f,right.z*aspect*0.72f);oglUniform3f(sUp,up.x*0.72f,up.y*0.72f,up.z*0.72f);oglUniform3f(sSunDir,sunDir.x,sunDir.y,sunDir.z);oglBindVertexArray(skyQuad.vao);glDrawElements(GL_TRIANGLES,skyQuad.indices,GL_UNSIGNED_INT,nullptr);glEnable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glPolygonMode(GL_FRONT_AND_BACK,app.wireframe?GL_LINE:GL_FILL);oglUseProgram(worldProgram);oglUniform3f(uSunDir,sunDir.x,sunDir.y,sunDir.z);oglUniform3f(uSunColor,sunColor.x,sunColor.y,sunColor.z);oglUniform3f(uSkyColor,skyColor.x,skyColor.y,skyColor.z);oglUniform3f(uCamera,app.camera.p.x,app.camera.p.y,app.camera.p.z);for(const Chunk& c:chunks)drawMesh(c.mesh,uModel,uMvp,uTint,proj,view,identity(),{1.0f,1.0f,1.0f});drawHotbar(uiMesh,uiProgram,app,w,h);glfwSwapBuffers(window);}destroyMesh(skyQuad);clearChunks(chunks);glfwTerminate();return 0;}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;glfwTerminate();return 1;}}
