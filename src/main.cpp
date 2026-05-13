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
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
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
#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER 0x8D40
#endif
#ifndef GL_DEPTH_ATTACHMENT
#define GL_DEPTH_ATTACHMENT 0x8D00
#endif
#ifndef GL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#endif
#ifndef GL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT24 0x81A6
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif
#ifndef GL_NONE
#define GL_NONE 0
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
constexpr int WorldHeight=64;
constexpr int ChunkRadius=3;
constexpr int WaterLevel=14;
constexpr int HotbarSlots=9;
constexpr int InventorySlots=27;
struct Vec3{float x;float y;float z;};
struct Mat4{float m[16];};
struct Vertex{Vec3 p;Vec3 n;Vec3 c;float t;};
struct Mesh{GLuint vao=0;GLuint vbo=0;GLuint ebo=0;GLsizei indices=0;};
struct ShadowMap{GLuint fbo=0;GLuint texture=0;int size=1024;};
struct Chunk{int cx=0;int cz=0;std::vector<unsigned char> blocks;Mesh mesh;Mesh waterMesh;bool dirty=true;};
struct RayHit{bool hit=false;int x=0;int y=0;int z=0;int px=0;int py=0;int pz=0;unsigned char type=0;};
struct Slot{unsigned char type=0;int count=0;float wear=1.0f;};
struct Camera{Vec3 p{0.0f,22.0f,0.0f};float yaw=-90.0f;float pitch=-18.0f;double lastX=640.0;double lastY=360.0;bool firstMouse=true;};
struct Player{Vec3 p{0.0f,24.0f,0.0f};Vec3 v{0.0f,0.0f,0.0f};float health=1.0f;float stamina=1.0f;bool grounded=false;bool swimming=false;};
struct App{Camera camera;Player player;bool wireframe=false;bool lastWire=false;bool lastReset=false;bool lastPlace=false;bool lastSort=false;bool lastTab=false;bool lastDistUp=false;bool lastDistDown=false;bool inventoryOpen=false;bool lastMouseLeft=false;int selected=0;int renderDistance=ChunkRadius;int dragSlot=-1;int mouseDownSlot=-1;float mouseHold=0.0f;double mouseX=640.0;double mouseY=360.0;Slot inventory[InventorySlots]{{1,24,1.0f},{2,24,1.0f},{3,18,1.0f},{4,8,1.0f},{5,12,1.0f},{6,8,1.0f},{7,16,1.0f},{8,18,1.0f},{0,0,1.0f}};bool lastSlot[HotbarSlots]{};int breakX=0;int breakY=0;int breakZ=0;float breakTimer=0.0f;float breakProgress=0.0f;std::vector<Chunk>* chunks=nullptr;};
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
using GlGenFramebuffers=void(APIENTRY*)(GLsizei,GLuint*);
using GlBindFramebuffer=void(APIENTRY*)(GLenum,GLuint);
using GlFramebufferTexture2D=void(APIENTRY*)(GLenum,GLenum,GLenum,GLuint,GLint);
using GlCheckFramebufferStatus=GLenum(APIENTRY*)(GLenum);
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
using GlUniform1f=void(APIENTRY*)(GLint,GLfloat);
using GlUniform1i=void(APIENTRY*)(GLint,GLint);
using GlActiveTexture=void(APIENTRY*)(GLenum);
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
static GlGenFramebuffers oglGenFramebuffers=nullptr;
static GlBindFramebuffer oglBindFramebuffer=nullptr;
static GlFramebufferTexture2D oglFramebufferTexture2D=nullptr;
static GlCheckFramebufferStatus oglCheckFramebufferStatus=nullptr;
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
static GlUniform1f oglUniform1f=nullptr;
static GlUniform1i oglUniform1i=nullptr;
static GlActiveTexture oglActiveTexture=nullptr;
static Vec3 operator+(Vec3 a,Vec3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
static Vec3 operator-(Vec3 a,Vec3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
static Vec3 operator*(Vec3 a,float s){return {a.x*s,a.y*s,a.z*s};}
static Vec3 operator/(Vec3 a,float s){return {a.x/s,a.y/s,a.z/s};}
static Vec3& operator+=(Vec3& a,Vec3 b){a=a+b;return a;}
static float dot(Vec3 a,Vec3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
static Vec3 cross(Vec3 a,Vec3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
static float len(Vec3 v){return std::sqrt(dot(v,v));}
static Vec3 norm(Vec3 v){float l=len(v);return l>0.00001f?v/l:Vec3{0.0f,1.0f,0.0f};}
static float fract(float v){return v-std::floor(v);}
static float smooth(float t){return t*t*(3.0f-2.0f*t);}
static float rand2(int x,int z){return fract(std::sin(static_cast<float>(x)*127.1f+static_cast<float>(z)*311.7f)*43758.5453f)*2.0f-1.0f;}
static float noise(float x,float z){int xi=static_cast<int>(std::floor(x));int zi=static_cast<int>(std::floor(z));float xf=x-static_cast<float>(xi);float zf=z-static_cast<float>(zi);float u=smooth(xf);float v=smooth(zf);float a=rand2(xi,zi);float b=rand2(xi+1,zi);float c=rand2(xi,zi+1);float d=rand2(xi+1,zi+1);return (a*(1.0f-u)+b*u)*(1.0f-v)+(c*(1.0f-u)+d*u)*v;}
static bool isWater(unsigned char type){return type==6;}
static bool isSolid(unsigned char type){return type!=0&&type!=6;}
static Vec3 itemColor(unsigned char type){Vec3 colors[9]{{0.04f,0.05f,0.06f},{0.30f,0.58f,0.20f},{0.45f,0.31f,0.18f},{0.43f,0.43f,0.42f},{0.86f,0.88f,0.84f},{0.47f,0.30f,0.14f},{0.20f,0.52f,0.85f},{0.25f,0.55f,0.24f},{0.72f,0.64f,0.39f}};return colors[std::clamp(static_cast<int>(type),0,8)];}
static Vec3 blockColor(unsigned char type,int y){Vec3 c=itemColor(type);float shade=0.82f+fract(static_cast<float>(y)*0.137f)*0.18f;return c*shade;}
static float blockHardness(unsigned char type){float h[9]{0.0f,0.55f,0.65f,1.45f,0.45f,0.95f,0.35f,0.30f,0.50f};return h[std::clamp(static_cast<int>(type),0,8)];}
static Mat4 identity(){Mat4 r{};r.m[0]=1.0f;r.m[5]=1.0f;r.m[10]=1.0f;r.m[15]=1.0f;return r;}
static Mat4 mul(Mat4 a,Mat4 b){Mat4 r{};for(int c=0;c<4;c++){for(int row=0;row<4;row++){r.m[c*4+row]=a.m[0*4+row]*b.m[c*4+0]+a.m[1*4+row]*b.m[c*4+1]+a.m[2*4+row]*b.m[c*4+2]+a.m[3*4+row]*b.m[c*4+3];}}return r;}
static Mat4 perspective(float fov,float aspect,float nearPlane,float farPlane){Mat4 r{};float t=std::tan(fov*0.5f);r.m[0]=1.0f/(aspect*t);r.m[5]=1.0f/t;r.m[10]=-(farPlane+nearPlane)/(farPlane-nearPlane);r.m[11]=-1.0f;r.m[14]=-(2.0f*farPlane*nearPlane)/(farPlane-nearPlane);return r;}
static Mat4 ortho(float l,float r,float b,float t,float n,float f){Mat4 m{};m.m[0]=2.0f/(r-l);m.m[5]=2.0f/(t-b);m.m[10]=-2.0f/(f-n);m.m[12]=-(r+l)/(r-l);m.m[13]=-(t+b)/(t-b);m.m[14]=-(f+n)/(f-n);m.m[15]=1.0f;return m;}
static Mat4 lookAt(Vec3 eye,Vec3 center,Vec3 up){Vec3 f=norm(center-eye);Vec3 s=norm(cross(f,up));Vec3 u=cross(s,f);Mat4 r=identity();r.m[0]=s.x;r.m[4]=s.y;r.m[8]=s.z;r.m[1]=u.x;r.m[5]=u.y;r.m[9]=u.z;r.m[2]=-f.x;r.m[6]=-f.y;r.m[10]=-f.z;r.m[12]=-dot(s,eye);r.m[13]=-dot(u,eye);r.m[14]=dot(f,eye);return r;}
static Vec3 cameraForward(const Camera& c){float yaw=c.yaw*Pi/180.0f;float pitch=c.pitch*Pi/180.0f;return norm({std::cos(yaw)*std::cos(pitch),std::sin(pitch),std::sin(yaw)*std::cos(pitch)});}
template<class T>static T loadProc(const char* name){auto proc=glfwGetProcAddress(name);if(!proc)throw std::runtime_error(std::string("OpenGL function not found: ")+name);return reinterpret_cast<T>(proc);}
static void loadGl(){oglCreateShader=loadProc<GlCreateShader>("glCreateShader");oglShaderSource=loadProc<GlShaderSource>("glShaderSource");oglCompileShader=loadProc<GlCompileShader>("glCompileShader");oglGetShaderiv=loadProc<GlGetShaderiv>("glGetShaderiv");oglGetShaderInfoLog=loadProc<GlGetShaderInfoLog>("glGetShaderInfoLog");oglCreateProgram=loadProc<GlCreateProgram>("glCreateProgram");oglAttachShader=loadProc<GlAttachShader>("glAttachShader");oglLinkProgram=loadProc<GlLinkProgram>("glLinkProgram");oglGetProgramiv=loadProc<GlGetProgramiv>("glGetProgramiv");oglGetProgramInfoLog=loadProc<GlGetProgramInfoLog>("glGetProgramInfoLog");oglDeleteShader=loadProc<GlDeleteShader>("glDeleteShader");oglUseProgram=loadProc<GlUseProgram>("glUseProgram");oglGenVertexArrays=loadProc<GlGenVertexArrays>("glGenVertexArrays");oglGenFramebuffers=loadProc<GlGenFramebuffers>("glGenFramebuffers");oglBindFramebuffer=loadProc<GlBindFramebuffer>("glBindFramebuffer");oglFramebufferTexture2D=loadProc<GlFramebufferTexture2D>("glFramebufferTexture2D");oglCheckFramebufferStatus=loadProc<GlCheckFramebufferStatus>("glCheckFramebufferStatus");oglDeleteVertexArrays=loadProc<GlDeleteVertexArrays>("glDeleteVertexArrays");oglBindVertexArray=loadProc<GlBindVertexArray>("glBindVertexArray");oglGenBuffers=loadProc<GlGenBuffers>("glGenBuffers");oglDeleteBuffers=loadProc<GlDeleteBuffers>("glDeleteBuffers");oglBindBuffer=loadProc<GlBindBuffer>("glBindBuffer");oglBufferData=loadProc<GlBufferData>("glBufferData");oglEnableVertexAttribArray=loadProc<GlEnableVertexAttribArray>("glEnableVertexAttribArray");oglVertexAttribPointer=loadProc<GlVertexAttribPointer>("glVertexAttribPointer");oglGetUniformLocation=loadProc<GlGetUniformLocation>("glGetUniformLocation");oglUniformMatrix4fv=loadProc<GlUniformMatrix4fv>("glUniformMatrix4fv");oglUniform3f=loadProc<GlUniform3f>("glUniform3f");oglUniform1f=loadProc<GlUniform1f>("glUniform1f");oglUniform1i=loadProc<GlUniform1i>("glUniform1i");oglActiveTexture=loadProc<GlActiveTexture>("glActiveTexture");}
static GLuint compileShader(GLenum type,const char* source){GLuint shader=oglCreateShader(type);oglShaderSource(shader,1,&source,nullptr);oglCompileShader(shader);GLint ok=0;oglGetShaderiv(shader,GL_COMPILE_STATUS,&ok);if(!ok){GLint size=0;oglGetShaderiv(shader,GL_INFO_LOG_LENGTH,&size);std::string log(static_cast<size_t>(std::max(size,1)),'\0');oglGetShaderInfoLog(shader,size,nullptr,log.data());throw std::runtime_error(log);}return shader;}
static GLuint createProgram(const char* vs,const char* fs){GLuint v=compileShader(GL_VERTEX_SHADER,vs);GLuint f=compileShader(GL_FRAGMENT_SHADER,fs);GLuint p=oglCreateProgram();oglAttachShader(p,v);oglAttachShader(p,f);oglLinkProgram(p);GLint ok=0;oglGetProgramiv(p,GL_LINK_STATUS,&ok);if(!ok){GLint size=0;oglGetProgramiv(p,GL_INFO_LOG_LENGTH,&size);std::string log(static_cast<size_t>(std::max(size,1)),'\0');oglGetProgramInfoLog(p,size,nullptr,log.data());throw std::runtime_error(log);}oglDeleteShader(v);oglDeleteShader(f);return p;}
static void destroyMesh(Mesh& mesh){if(mesh.ebo)oglDeleteBuffers(1,&mesh.ebo);if(mesh.vbo)oglDeleteBuffers(1,&mesh.vbo);if(mesh.vao)oglDeleteVertexArrays(1,&mesh.vao);mesh={};}
static Mesh uploadMesh(const std::vector<Vertex>& vertices,const std::vector<unsigned int>& indices){Mesh mesh;mesh.indices=static_cast<GLsizei>(indices.size());oglGenVertexArrays(1,&mesh.vao);oglGenBuffers(1,&mesh.vbo);oglGenBuffers(1,&mesh.ebo);oglBindVertexArray(mesh.vao);oglBindBuffer(GL_ARRAY_BUFFER,mesh.vbo);oglBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size()*sizeof(Vertex)),vertices.empty()?nullptr:vertices.data(),GL_STATIC_DRAW);oglBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh.ebo);oglBufferData(GL_ELEMENT_ARRAY_BUFFER,static_cast<GLsizeiptr>(indices.size()*sizeof(unsigned int)),indices.empty()?nullptr:indices.data(),GL_STATIC_DRAW);oglEnableVertexAttribArray(0);oglVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,p)));oglEnableVertexAttribArray(1);oglVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,n)));oglEnableVertexAttribArray(2);oglVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,c)));oglEnableVertexAttribArray(3);oglVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,t)));oglBindVertexArray(0);return mesh;}
static Mesh createScreenQuad(){std::vector<Vertex> vertices{{{-1.0f,-1.0f,0.0f},{0.0f,0.0f,1.0f},{1.0f,1.0f,1.0f},0.0f},{{1.0f,-1.0f,0.0f},{0.0f,0.0f,1.0f},{1.0f,1.0f,1.0f},0.0f},{{-1.0f,1.0f,0.0f},{0.0f,0.0f,1.0f},{1.0f,1.0f,1.0f},0.0f},{{1.0f,1.0f,0.0f},{0.0f,0.0f,1.0f},{1.0f,1.0f,1.0f},0.0f}};std::vector<unsigned int> indices{0,1,2,2,1,3};return uploadMesh(vertices,indices);}
static ShadowMap createShadowMap(int size){ShadowMap sm;sm.size=size;glGenTextures(1,&sm.texture);glBindTexture(GL_TEXTURE_2D,sm.texture);glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,size,size,0,GL_DEPTH_COMPONENT,GL_FLOAT,nullptr);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);oglGenFramebuffers(1,&sm.fbo);oglBindFramebuffer(GL_FRAMEBUFFER,sm.fbo);oglFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,sm.texture,0);glDrawBuffer(GL_NONE);glReadBuffer(GL_NONE);if(oglCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)throw std::runtime_error("Shadow framebuffer failed");oglBindFramebuffer(GL_FRAMEBUFFER,0);return sm;}
static int divFloor(int a,int b){int q=a/b;int r=a%b;return r&&((r<0)!=(b<0))?q-1:q;}
static int modFloor(int a,int b){int r=a%b;return r<0?r+b:r;}
static int blockIndex(int x,int y,int z){return (y*ChunkSize+z)*ChunkSize+x;}
static int blockHeight(int x,int z){float fx=static_cast<float>(x);float fz=static_cast<float>(z);float broad=noise(fx*0.010f-70.0f,fz*0.010f+20.0f);float hills=noise(fx*0.032f+30.0f,fz*0.032f-12.0f);float detail=noise(fx*0.078f,fz*0.078f);float ridge=1.0f-std::abs(noise(fx*0.020f+90.0f,fz*0.020f-110.0f));float h=22.0f+broad*17.0f+hills*9.0f+detail*2.6f+ridge*ridge*8.0f;return std::clamp(static_cast<int>(std::floor(h)),5,WorldHeight-8);}
static Chunk* findChunk(std::vector<Chunk>& chunks,int cx,int cz){for(Chunk& c:chunks)if(c.cx==cx&&c.cz==cz)return &c;return nullptr;}
static const Chunk* findChunkConst(const std::vector<Chunk>& chunks,int cx,int cz){for(const Chunk& c:chunks)if(c.cx==cx&&c.cz==cz)return &c;return nullptr;}
static unsigned char getBlock(const std::vector<Chunk>& chunks,int wx,int y,int wz){if(y<0||y>=WorldHeight)return 0;int cx=divFloor(wx,ChunkSize);int cz=divFloor(wz,ChunkSize);int lx=modFloor(wx,ChunkSize);int lz=modFloor(wz,ChunkSize);const Chunk* c=findChunkConst(chunks,cx,cz);return c?c->blocks[blockIndex(lx,y,lz)]:0;}
static void markChunkDirty(std::vector<Chunk>& chunks,int cx,int cz){if(Chunk* c=findChunk(chunks,cx,cz))c->dirty=true;}
static void markBlockDirty(std::vector<Chunk>& chunks,int wx,int wz){int cx=divFloor(wx,ChunkSize);int cz=divFloor(wz,ChunkSize);int lx=modFloor(wx,ChunkSize);int lz=modFloor(wz,ChunkSize);markChunkDirty(chunks,cx,cz);if(lx==0)markChunkDirty(chunks,cx-1,cz);if(lx==ChunkSize-1)markChunkDirty(chunks,cx+1,cz);if(lz==0)markChunkDirty(chunks,cx,cz-1);if(lz==ChunkSize-1)markChunkDirty(chunks,cx,cz+1);}
static void setBlock(std::vector<Chunk>& chunks,int wx,int y,int wz,unsigned char type){if(y<0||y>=WorldHeight)return;int cx=divFloor(wx,ChunkSize);int cz=divFloor(wz,ChunkSize);int lx=modFloor(wx,ChunkSize);int lz=modFloor(wz,ChunkSize);Chunk* c=findChunk(chunks,cx,cz);if(c){c->blocks[blockIndex(lx,y,lz)]=type;markBlockDirty(chunks,wx,wz);}}
static Chunk makeChunk(int cx,int cz){Chunk c;c.cx=cx;c.cz=cz;c.blocks.assign(static_cast<size_t>(ChunkSize*WorldHeight*ChunkSize),0);for(int z=0;z<ChunkSize;z++){for(int x=0;x<ChunkSize;x++){int wx=cx*ChunkSize+x;int wz=cz*ChunkSize+z;int h=blockHeight(wx,wz);float dry=noise(static_cast<float>(wx)*0.020f+140.0f,static_cast<float>(wz)*0.020f-40.0f);for(int y=0;y<=h;y++){unsigned char type=3;if(y==h)type=h<=WaterLevel+1?8:(h>41?4:(dry<-0.48f?8:1));else if(y>h-4)type=h<=WaterLevel+1?8:2;if(y<10&&rand2(wx*17+y,wz*19)>0.84f)type=5;c.blocks[blockIndex(x,y,z)]=type;}if(h<WaterLevel){for(int y=h+1;y<=WaterLevel;y++)c.blocks[blockIndex(x,y,z)]=6;}if(h>WaterLevel+2&&h<WorldHeight-8&&rand2(wx*31,wz*37)>0.945f){for(int y=h+1;y<h+5;y++)c.blocks[blockIndex(x,y,z)]=5;for(int dz=-2;dz<=2;dz++){for(int dx=-2;dx<=2;dx++){for(int dy=3;dy<=6;dy++){int lx=x+dx;int lz=z+dz;int ly=h+dy;if(lx>=0&&lx<ChunkSize&&lz>=0&&lz<ChunkSize&&ly>=0&&ly<WorldHeight&&std::abs(dx)+std::abs(dz)+dy<9)c.blocks[blockIndex(lx,ly,lz)]=7;}}}}}}c.dirty=true;return c;}
static void addFace(std::vector<Vertex>& vertices,std::vector<unsigned int>& indices,float x,float y,float z,int face,Vec3 color,unsigned char type){static const Vec3 normals[6]{{1.0f,0.0f,0.0f},{-1.0f,0.0f,0.0f},{0.0f,1.0f,0.0f},{0.0f,-1.0f,0.0f},{0.0f,0.0f,1.0f},{0.0f,0.0f,-1.0f}};static const Vec3 p[6][4]{{{1,0,0},{1,1,0},{1,1,1},{1,0,1}},{{0,0,1},{0,1,1},{0,1,0},{0,0,0}},{{0,1,1},{1,1,1},{1,1,0},{0,1,0}},{{0,0,0},{1,0,0},{1,0,1},{0,0,1}},{{1,0,1},{1,1,1},{0,1,1},{0,0,1}},{{0,0,0},{0,1,0},{1,1,0},{1,0,0}}};unsigned int base=static_cast<unsigned int>(vertices.size());float light[6]{0.82f,0.68f,1.0f,0.50f,0.76f,0.62f};Vec3 c=color*light[face];for(int i=0;i<4;i++)vertices.push_back({{x+p[face][i].x,y+p[face][i].y,z+p[face][i].z},normals[face],c,static_cast<float>(type)});indices.push_back(base);indices.push_back(base+1);indices.push_back(base+2);indices.push_back(base);indices.push_back(base+2);indices.push_back(base+3);}
static void rebuildChunkMesh(std::vector<Chunk>& chunks,Chunk& chunk){std::vector<Vertex> solidVertices;std::vector<unsigned int> solidIndices;std::vector<Vertex> waterVertices;std::vector<unsigned int> waterIndices;solidVertices.reserve(ChunkSize*ChunkSize*256);waterVertices.reserve(ChunkSize*ChunkSize*32);int dirs[6][3]{{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};for(int y=0;y<WorldHeight;y++){for(int z=0;z<ChunkSize;z++){for(int x=0;x<ChunkSize;x++){unsigned char type=chunk.blocks[blockIndex(x,y,z)];if(!type)continue;int wx=chunk.cx*ChunkSize+x;int wz=chunk.cz*ChunkSize+z;if(isWater(type)){for(int f=0;f<6;f++){unsigned char n=getBlock(chunks,wx+dirs[f][0],y+dirs[f][1],wz+dirs[f][2]);if(n!=6)addFace(waterVertices,waterIndices,static_cast<float>(wx),static_cast<float>(y),static_cast<float>(wz),f,blockColor(type,y),type);}}else{for(int f=0;f<6;f++){unsigned char n=getBlock(chunks,wx+dirs[f][0],y+dirs[f][1],wz+dirs[f][2]);if(!isSolid(n))addFace(solidVertices,solidIndices,static_cast<float>(wx),static_cast<float>(y),static_cast<float>(wz),f,blockColor(type,y),type);}}}}}destroyMesh(chunk.mesh);destroyMesh(chunk.waterMesh);chunk.mesh=uploadMesh(solidVertices,solidIndices);chunk.waterMesh=uploadMesh(waterVertices,waterIndices);chunk.dirty=false;}
static void rebuildDirtyChunks(std::vector<Chunk>& chunks,int maxCount=9999){int done=0;for(size_t i=0;i<chunks.size()&&done<maxCount;i++){if(chunks[i].dirty){rebuildChunkMesh(chunks,chunks[i]);done++;}}}
static void clearChunks(std::vector<Chunk>& chunks){for(Chunk& c:chunks){destroyMesh(c.mesh);destroyMesh(c.waterMesh);}chunks.clear();}
static void ensureChunks(std::vector<Chunk>& chunks,Vec3 p,int radius=ChunkRadius){int bx=static_cast<int>(std::floor(p.x));int bz=static_cast<int>(std::floor(p.z));int pcx=divFloor(bx,ChunkSize);int pcz=divFloor(bz,ChunkSize);for(size_t i=0;i<chunks.size();){if(std::abs(chunks[i].cx-pcx)>radius+1||std::abs(chunks[i].cz-pcz)>radius+1){int cx=chunks[i].cx;int cz=chunks[i].cz;destroyMesh(chunks[i].mesh);destroyMesh(chunks[i].waterMesh);chunks.erase(chunks.begin()+static_cast<std::ptrdiff_t>(i));markChunkDirty(chunks,cx-1,cz);markChunkDirty(chunks,cx+1,cz);markChunkDirty(chunks,cx,cz-1);markChunkDirty(chunks,cx,cz+1);}else i++;}for(int z=pcz-radius;z<=pcz+radius;z++){for(int x=pcx-radius;x<=pcx+radius;x++){if(!findChunk(chunks,x,z)){chunks.push_back(makeChunk(x,z));markChunkDirty(chunks,x-1,z);markChunkDirty(chunks,x+1,z);markChunkDirty(chunks,x,z-1);markChunkDirty(chunks,x,z+1);}}}}
static bool treeAt(int x,int z){int h=blockHeight(x,z);return h>WaterLevel+2&&h<WorldHeight-8&&rand2(x*31,z*37)>0.945f;}
static Vec3 findSpawn(){for(int r=0;r<=128;r++){for(int z=-r;z<=r;z++){for(int x=-r;x<=r;x++){if(std::max(std::abs(x),std::abs(z))!=r)continue;int h=blockHeight(x,z);if(h<=WaterLevel+2)continue;bool clear=true;for(int dz=-3;dz<=3;dz++)for(int dx=-3;dx<=3;dx++)if(treeAt(x+dx,z+dz))clear=false;if(clear)return {static_cast<float>(x)+0.5f,static_cast<float>(h)+1.06f,static_cast<float>(z)+0.5f};}}}int h=std::max(blockHeight(0,0),WaterLevel+3);return {0.5f,static_cast<float>(h)+1.06f,0.5f};}
static bool playerHits(const std::vector<Chunk>& chunks,Vec3 p){float hw=0.32f;float h=1.78f;int minX=static_cast<int>(std::floor(p.x-hw));int maxX=static_cast<int>(std::floor(p.x+hw));int minY=static_cast<int>(std::floor(p.y+0.03f));int maxY=static_cast<int>(std::floor(p.y+h-0.03f));int minZ=static_cast<int>(std::floor(p.z-hw));int maxZ=static_cast<int>(std::floor(p.z+hw));for(int y=minY;y<=maxY;y++)for(int z=minZ;z<=maxZ;z++)for(int x=minX;x<=maxX;x++)if(isSolid(getBlock(chunks,x,y,z)))return true;return false;}
static bool blockTouchesPlayer(Vec3 p,int x,int y,int z){float hw=0.38f;float h=1.86f;return p.x+hw>static_cast<float>(x)&&p.x-hw<static_cast<float>(x+1)&&p.y+h>static_cast<float>(y)&&p.y<static_cast<float>(y+1)&&p.z+hw>static_cast<float>(z)&&p.z-hw<static_cast<float>(z+1);}
static void keepPlayerClear(App& app){if(!app.chunks)return;for(int i=0;i<24&&playerHits(*app.chunks,app.player.p);i++)app.player.p.y+=0.25f;}
static RayHit raycast(const std::vector<Chunk>& chunks,Vec3 origin,Vec3 dir){RayHit hit;int lastX=static_cast<int>(std::floor(origin.x));int lastY=static_cast<int>(std::floor(origin.y));int lastZ=static_cast<int>(std::floor(origin.z));for(float t=0.0f;t<6.5f;t+=0.05f){Vec3 p=origin+dir*t;int x=static_cast<int>(std::floor(p.x));int y=static_cast<int>(std::floor(p.y));int z=static_cast<int>(std::floor(p.z));unsigned char type=getBlock(chunks,x,y,z);if(isSolid(type)){hit.hit=true;hit.x=x;hit.y=y;hit.z=z;hit.px=lastX;hit.py=lastY;hit.pz=lastZ;hit.type=type;return hit;}lastX=x;lastY=y;lastZ=z;}return hit;}
static void resetInventory(App& app){for(Slot& s:app.inventory)s={0,0,1.0f};app.inventory[0]={1,24,1.0f};app.inventory[1]={2,24,1.0f};app.inventory[2]={3,18,1.0f};app.inventory[3]={4,8,1.0f};app.inventory[4]={5,12,1.0f};app.inventory[5]={6,8,1.0f};app.inventory[6]={7,16,1.0f};app.inventory[7]={8,18,1.0f};app.selected=0;}
static bool addItem(App& app,unsigned char type,int count){for(Slot& s:app.inventory){if(s.type==type&&s.count>0&&s.count<64){int take=std::min(count,64-s.count);s.count+=take;count-=take;if(count<=0)return true;}}for(Slot& s:app.inventory){if(!s.count){int take=std::min(count,64);s={type,take,1.0f};count-=take;if(count<=0)return true;}}return count<=0;}
static const char* blockName(unsigned char type){static const char* names[9]{"EMPTY","GRASS","DIRT","STONE","SNOW","WOOD","WATER","LEAVES","SAND"};return names[std::clamp(static_cast<int>(type),0,8)];}
static void sortInventory(App& app){std::sort(app.inventory,app.inventory+InventorySlots,[](const Slot& a,const Slot& b){if(a.count==0&&b.count==0)return false;if(a.count==0)return false;if(b.count==0)return true;if(a.type!=b.type)return a.type<b.type;return a.count>b.count;});app.selected=std::clamp(app.selected,0,HotbarSlots-1);}
static bool playerInWater(const std::vector<Chunk>& chunks,Vec3 p){int x=static_cast<int>(std::floor(p.x));int z=static_cast<int>(std::floor(p.z));int y0=static_cast<int>(std::floor(p.y+0.25f));int y1=static_cast<int>(std::floor(p.y+1.55f));return isWater(getBlock(chunks,x,y0,z))||isWater(getBlock(chunks,x,y1,z));}
static int inventorySlotAt(double mx,double my,int w,int h){float fw=static_cast<float>(std::max(w,1));float fh=static_cast<float>(std::max(h,1));float slot=std::clamp(fh*0.080f,52.0f,72.0f);float gap=slot*0.12f;float total=slot*HotbarSlots+gap*(HotbarSlots-1);float hotX=(fw-total)*0.5f;float hotY=fh-slot-28.0f;float invSlot=slot*0.95f;float invGap=invSlot*0.16f;float invTotal=invSlot*HotbarSlots+invGap*(HotbarSlots-1);float invX=(fw-invTotal)*0.5f;float invY=fh*0.16f+48.0f;float x=static_cast<float>(mx);float y=static_cast<float>(my);for(int row=0;row<3;row++){for(int col=0;col<HotbarSlots;col++){float x0=invX+static_cast<float>(col)*(invSlot+invGap);float y0=invY+static_cast<float>(row)*(invSlot+invGap);if(x>=x0&&x<=x0+invSlot&&y>=y0&&y<=y0+invSlot)return row*HotbarSlots+col;}}for(int col=0;col<HotbarSlots;col++){float x0=hotX+static_cast<float>(col)*(slot+gap);if(x>=x0&&x<=x0+slot&&y>=hotY&&y<=hotY+slot)return col;}return -1;}
static void updateInventoryMouse(GLFWwindow* window,App& app,float dt){int w=0;int h=0;glfwGetFramebufferSize(window,&w,&h);bool down=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;if(down&&!app.lastMouseLeft){app.mouseDownSlot=inventorySlotAt(app.mouseX,app.mouseY,w,h);app.dragSlot=-1;app.mouseHold=0.0f;}if(down&&app.mouseDownSlot>=0){app.mouseHold+=dt;if(app.mouseHold>0.14f)app.dragSlot=app.mouseDownSlot;}if(!down&&app.lastMouseLeft){int slot=inventorySlotAt(app.mouseX,app.mouseY,w,h);if(app.dragSlot>=0&&slot>=0&&slot<InventorySlots&&slot!=app.dragSlot)std::swap(app.inventory[app.dragSlot],app.inventory[slot]);else if(slot>=0&&slot<HotbarSlots)app.selected=slot;app.dragSlot=-1;app.mouseDownSlot=-1;app.mouseHold=0.0f;}app.lastMouseLeft=down;}
static void scrollCallback(GLFWwindow* window,double xoffset,double yoffset){if(ImGui::GetCurrentContext())ImGui_ImplGlfw_ScrollCallback(window,xoffset,yoffset);auto* app=static_cast<App*>(glfwGetWindowUserPointer(window));if(!app||app->inventoryOpen)return;if(yoffset>0.0)app->selected=(app->selected+HotbarSlots-1)%HotbarSlots;if(yoffset<0.0)app->selected=(app->selected+1)%HotbarSlots;}
static void cursorCallback(GLFWwindow* window,double xpos,double ypos){if(ImGui::GetCurrentContext())ImGui_ImplGlfw_CursorPosCallback(window,xpos,ypos);auto* app=static_cast<App*>(glfwGetWindowUserPointer(window));if(!app)return;app->mouseX=xpos;app->mouseY=ypos;if(app->inventoryOpen){app->camera.firstMouse=true;return;}Camera& c=app->camera;if(c.firstMouse){c.lastX=xpos;c.lastY=ypos;c.firstMouse=false;}float dx=static_cast<float>(xpos-c.lastX);float dy=static_cast<float>(c.lastY-ypos);c.lastX=xpos;c.lastY=ypos;float sens=0.09f;c.yaw+=dx*sens;c.pitch=std::clamp(c.pitch+dy*sens,-84.0f,84.0f);}
static bool pressed(GLFWwindow* window,int key){return glfwGetKey(window,key)==GLFW_PRESS;}
static void moveAxis(App& app,float dx,float dy,float dz,bool step){if(!app.chunks)return;Vec3 old=app.player.p;Vec3 next{old.x+dx,old.y+dy,old.z+dz};if(!playerHits(*app.chunks,next)){app.player.p=next;return;}if(step&&(dx!=0.0f||dz!=0.0f)){Vec3 up{old.x+dx,old.y+1.02f,old.z+dz};if(!playerHits(*app.chunks,up)){app.player.p=up;return;}}if(dx!=0.0f)app.player.v.x=0.0f;if(dy!=0.0f){if(dy<0.0f)app.player.grounded=true;app.player.v.y=0.0f;}if(dz!=0.0f)app.player.v.z=0.0f;}
static void updateInput(GLFWwindow* window,App& app,float dt){
if(!app.chunks)return;
if(pressed(window,GLFW_KEY_ESCAPE))glfwSetWindowShouldClose(window,GLFW_TRUE);
bool tabNow=pressed(window,GLFW_KEY_TAB);
if(tabNow&&!app.lastTab){app.inventoryOpen=!app.inventoryOpen;app.dragSlot=-1;app.mouseDownSlot=-1;app.lastMouseLeft=false;glfwSetInputMode(window,GLFW_CURSOR,app.inventoryOpen?GLFW_CURSOR_NORMAL:GLFW_CURSOR_DISABLED);app.camera.firstMouse=true;}
app.lastTab=tabNow;
bool sortNow=pressed(window,GLFW_KEY_T);
if(sortNow&&!app.lastSort)sortInventory(app);
app.lastSort=sortNow;
bool distDown=pressed(window,GLFW_KEY_LEFT_BRACKET);
if(distDown&&!app.lastDistDown){app.renderDistance=std::max(2,app.renderDistance-1);ensureChunks(*app.chunks,app.player.p,app.renderDistance);}
app.lastDistDown=distDown;
bool distUp=pressed(window,GLFW_KEY_RIGHT_BRACKET);
if(distUp&&!app.lastDistUp){app.renderDistance=std::min(5,app.renderDistance+1);ensureChunks(*app.chunks,app.player.p,app.renderDistance);}
app.lastDistUp=distUp;
bool wireNow=pressed(window,GLFW_KEY_F);
if(wireNow&&!app.lastWire)app.wireframe=!app.wireframe;
app.lastWire=wireNow;
bool resetNow=pressed(window,GLFW_KEY_R);
if(resetNow&&!app.lastReset){clearChunks(*app.chunks);resetInventory(app);app.player.p=findSpawn();app.player.v={0.0f,0.0f,0.0f};app.player.health=1.0f;app.player.stamina=1.0f;app.breakTimer=0.0f;app.breakProgress=0.0f;ensureChunks(*app.chunks,app.player.p,app.renderDistance);keepPlayerClear(app);}
app.lastReset=resetNow;
if(app.inventoryOpen){updateInventoryMouse(window,app,dt);app.player.v.x=0.0f;app.player.v.z=0.0f;app.breakTimer=0.0f;app.breakProgress=0.0f;app.camera.p=app.player.p+Vec3{0.0f,1.62f,0.0f};return;}
int keys[HotbarSlots]{GLFW_KEY_1,GLFW_KEY_2,GLFW_KEY_3,GLFW_KEY_4,GLFW_KEY_5,GLFW_KEY_6,GLFW_KEY_7,GLFW_KEY_8,GLFW_KEY_9};
for(int i=0;i<HotbarSlots;i++){bool down=pressed(window,keys[i]);if(down&&!app.lastSlot[i])app.selected=i;app.lastSlot[i]=down;}
Vec3 eye=app.player.p+Vec3{0.0f,1.62f,0.0f};
Vec3 dir=cameraForward(app.camera);
bool breakNow=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;
if(breakNow){RayHit hit=raycast(*app.chunks,eye,dir);if(hit.hit&&hit.y>0){if(hit.x!=app.breakX||hit.y!=app.breakY||hit.z!=app.breakZ){app.breakX=hit.x;app.breakY=hit.y;app.breakZ=hit.z;app.breakTimer=0.0f;}float hardness=std::max(0.1f,blockHardness(hit.type));app.breakTimer+=dt;app.breakProgress=std::clamp(app.breakTimer/hardness,0.0f,1.0f);if(app.breakProgress>=1.0f){setBlock(*app.chunks,hit.x,hit.y,hit.z,0);addItem(app,hit.type,1);app.breakTimer=0.0f;app.breakProgress=0.0f;}}else{app.breakTimer=0.0f;app.breakProgress=0.0f;}}else{app.breakTimer=0.0f;app.breakProgress=0.0f;}
bool placeNow=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS;
if(placeNow&&!app.lastPlace){RayHit hit=raycast(*app.chunks,eye,dir);Slot& slot=app.inventory[app.selected];if(hit.hit&&slot.count>0&&slot.type>0&&hit.py>=0&&hit.py<WorldHeight&&!isSolid(getBlock(*app.chunks,hit.px,hit.py,hit.pz))&&!blockTouchesPlayer(app.player.p,hit.px,hit.py,hit.pz)){setBlock(*app.chunks,hit.px,hit.py,hit.pz,slot.type);slot.count--;slot.wear=std::max(0.0f,slot.wear-0.03f);if(slot.count<=0)slot={0,0,1.0f};}}
app.lastPlace=placeNow;
Vec3 f=dir;
Vec3 flat=norm({f.x,0.0f,f.z});
Vec3 right=norm(cross(flat,{0.0f,1.0f,0.0f}));
Vec3 wish{0.0f,0.0f,0.0f};
if(pressed(window,GLFW_KEY_W))wish+=flat;
if(pressed(window,GLFW_KEY_S))wish+=flat*-1.0f;
if(pressed(window,GLFW_KEY_D))wish+=right;
if(pressed(window,GLFW_KEY_A))wish+=right*-1.0f;
bool moving=len(wish)>0.01f;
if(moving)wish=norm(wish);
app.player.swimming=playerInWater(*app.chunks,app.player.p);
bool sprintKey=pressed(window,GLFW_KEY_LEFT_SHIFT)||pressed(window,GLFW_KEY_RIGHT_SHIFT);
bool sprint=sprintKey&&moving&&app.player.stamina>0.06f;
float staminaUse=app.player.swimming?0.14f:0.22f;
float staminaGain=app.player.grounded?0.24f:0.12f;
if(sprint)app.player.stamina=std::max(0.0f,app.player.stamina-staminaUse*dt);
else app.player.stamina=std::min(1.0f,app.player.stamina+staminaGain*dt);
float speed=app.player.swimming?(sprint?4.5f:3.2f):(sprint?7.2f:4.8f);
app.player.v.x=wish.x*speed;
app.player.v.z=wish.z*speed;
if(app.player.swimming){if(pressed(window,GLFW_KEY_SPACE))app.player.v.y+=10.0f*dt;if(pressed(window,GLFW_KEY_LEFT_CONTROL)||pressed(window,GLFW_KEY_RIGHT_CONTROL))app.player.v.y-=8.0f*dt;app.player.v.y=std::clamp(app.player.v.y-3.0f*dt,-3.8f,3.2f);app.player.grounded=false;}
else{if(pressed(window,GLFW_KEY_SPACE)&&app.player.grounded){app.player.v.y=8.2f;app.player.grounded=false;}app.player.v.y-=22.0f*dt;}
bool canStep=app.player.grounded&&!app.player.swimming;
app.player.grounded=false;
moveAxis(app,app.player.v.x*dt,0.0f,0.0f,canStep);
moveAxis(app,0.0f,0.0f,app.player.v.z*dt,canStep);
moveAxis(app,0.0f,app.player.v.y*dt,0.0f,false);
if(app.player.p.y<-16.0f){app.player.health=std::max(0.2f,app.player.health-0.15f);app.player.p=findSpawn();ensureChunks(*app.chunks,app.player.p,app.renderDistance);keepPlayerClear(app);app.player.v={0.0f,0.0f,0.0f};}
app.camera.p=app.player.p+Vec3{0.0f,1.62f,0.0f};
}
static ImU32 imguiColor(Vec3 c,float a=1.0f){return ImGui::ColorConvertFloat4ToU32({std::clamp(c.x,0.0f,1.0f),std::clamp(c.y,0.0f,1.0f),std::clamp(c.z,0.0f,1.0f),a});}
static void drawImGuiBar(ImDrawList* d,ImVec2 p,ImVec2 q,float value,ImU32 bg,ImU32 fg){float v=std::clamp(value,0.0f,1.0f);d->AddRectFilled(p,q,bg,6.0f);d->AddRectFilled(p,{p.x+(q.x-p.x)*v,q.y},fg,6.0f);d->AddRect(p,q,imguiColor({0.02f,0.025f,0.03f}),6.0f,0,2.0f);}
static void drawImGuiIcon(ImDrawList* d,ImVec2 p,ImVec2 q,unsigned char type){Vec3 c=itemColor(type);float w=q.x-p.x;float h=q.y-p.y;d->AddRectFilled(p,q,imguiColor(c*0.82f),5.0f);d->AddRectFilled({p.x+w*0.07f,p.y+h*0.08f},{q.x-w*0.07f,q.y-h*0.08f},imguiColor(c*1.10f),4.0f);d->AddRectFilled({p.x+w*0.07f,p.y+h*0.08f},{q.x-w*0.07f,p.y+h*0.22f},imguiColor(c*1.26f),4.0f);if(type==1){d->AddRectFilled({p.x+w*0.12f,p.y+h*0.10f},{q.x-w*0.12f,p.y+h*0.34f},imguiColor({0.18f,0.74f,0.20f}),3.0f);d->AddRectFilled({p.x+w*0.16f,p.y+h*0.60f},{q.x-w*0.16f,p.y+h*0.78f},imguiColor({0.38f,0.24f,0.12f}),2.0f);}else if(type==2){for(int i=0;i<4;i++)d->AddRectFilled({p.x+w*(0.14f+0.19f*i),p.y+h*0.22f},{p.x+w*(0.22f+0.19f*i),q.y-h*0.18f},imguiColor({0.30f,0.18f,0.10f}),2.0f);}else if(type==3){d->AddRectFilled({p.x+w*0.17f,p.y+h*0.18f},{p.x+w*0.46f,p.y+h*0.48f},imguiColor({0.25f,0.26f,0.26f}),3.0f);d->AddRectFilled({p.x+w*0.55f,p.y+h*0.52f},{p.x+w*0.84f,p.y+h*0.80f},imguiColor({0.60f,0.60f,0.58f}),3.0f);}else if(type==4){d->AddRectFilled({p.x+w*0.10f,p.y+h*0.16f},{q.x-w*0.10f,p.y+h*0.50f},imguiColor({1.0f,1.0f,0.96f}),4.0f);d->AddRectFilled({p.x+w*0.18f,p.y+h*0.58f},{q.x-w*0.20f,p.y+h*0.72f},imguiColor({0.80f,0.86f,0.90f}),2.0f);}else if(type==5){for(int i=0;i<5;i++)d->AddRectFilled({p.x+w*0.13f,p.y+h*(0.18f+0.14f*i)},{q.x-w*0.13f,p.y+h*(0.23f+0.14f*i)},imguiColor({0.25f,0.14f,0.06f}),2.0f);d->AddRectFilled({p.x+w*0.46f,p.y+h*0.13f},{p.x+w*0.56f,q.y-h*0.13f},imguiColor({0.58f,0.36f,0.16f}),2.0f);}else if(type==6){d->AddRectFilled({p.x+w*0.08f,p.y+h*0.14f},{q.x-w*0.08f,q.y-h*0.12f},imguiColor({0.12f,0.48f,0.86f}),5.0f);for(int i=0;i<4;i++)d->AddLine({p.x+w*0.15f,p.y+h*(0.30f+0.13f*i)},{q.x-w*0.14f,p.y+h*(0.25f+0.13f*i)},imguiColor({0.58f,0.88f,1.0f}),2.0f);}else if(type==7){d->AddRectFilled({p.x+w*0.13f,p.y+h*0.18f},{p.x+w*0.52f,p.y+h*0.55f},imguiColor({0.14f,0.62f,0.17f}),5.0f);d->AddRectFilled({p.x+w*0.42f,p.y+h*0.40f},{p.x+w*0.86f,p.y+h*0.84f},imguiColor({0.10f,0.44f,0.12f}),5.0f);}else if(type==8){for(int i=0;i<4;i++)d->AddRectFilled({p.x+w*(0.13f+0.18f*i),p.y+h*0.28f},{p.x+w*(0.22f+0.18f*i),p.y+h*0.43f},imguiColor({0.94f,0.84f,0.50f}),2.0f);d->AddRectFilled({p.x+w*0.12f,p.y+h*0.60f},{q.x-w*0.12f,p.y+h*0.72f},imguiColor({0.62f,0.54f,0.32f}),2.0f);}}
static void drawImGuiSlot(ImDrawList* d,const Slot& s,ImVec2 p,float size,bool selected){ImVec2 q{p.x+size,p.y+size};d->AddRectFilled({p.x+4.0f,p.y+5.0f},{q.x+5.0f,q.y+6.0f},imguiColor({0.01f,0.012f,0.016f},0.55f),8.0f);d->AddRectFilled(p,q,selected?imguiColor({0.16f,0.18f,0.21f},0.94f):imguiColor({0.055f,0.065f,0.080f},0.90f),8.0f);d->AddRect(p,q,selected?imguiColor({1.0f,0.82f,0.26f}):imguiColor({0.22f,0.26f,0.31f}),8.0f,0,selected?3.0f:1.4f);ImVec2 ip{p.x+size*0.13f,p.y+size*0.13f};ImVec2 iq{q.x-size*0.13f,q.y-size*0.13f};if(s.count>0)drawImGuiIcon(d,ip,iq,s.type);if(s.count>0){drawImGuiBar(d,{p.x+size*0.14f,q.y-size*0.11f},{q.x-size*0.14f,q.y-size*0.06f},s.wear,imguiColor({0.05f,0.06f,0.07f}),s.wear>0.35f?imguiColor({0.28f,0.92f,0.42f}):imguiColor({0.92f,0.34f,0.22f}));if(s.count>1){std::string count=std::to_string(s.count);ImVec2 ts=ImGui::CalcTextSize(count.c_str());d->AddText({q.x-ts.x-7.0f,q.y-ts.y-8.0f},imguiColor({0.0f,0.0f,0.0f}),count.c_str());d->AddText({q.x-ts.x-8.0f,q.y-ts.y-9.0f},imguiColor({1.0f,0.96f,0.76f}),count.c_str());}}}
static void drawImGuiHud(const App& app,int w,int h){ImGui::SetNextWindowPos({0.0f,0.0f});ImGui::SetNextWindowSize({static_cast<float>(w),static_cast<float>(h)});ImGui::Begin("WorldOverlay",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoInputs|ImGuiWindowFlags_NoBackground);ImDrawList* d=ImGui::GetWindowDrawList();float slot=std::clamp(static_cast<float>(h)*0.080f,52.0f,72.0f);float gap=slot*0.12f;float total=slot*HotbarSlots+gap*(HotbarSlots-1);float start=(static_cast<float>(w)-total)*0.5f;float y=static_cast<float>(h)-slot-28.0f;for(int i=0;i<HotbarSlots;i++)drawImGuiSlot(d,app.inventory[i],{start+static_cast<float>(i)*(slot+gap),y},slot,i==app.selected);ImVec2 c{static_cast<float>(w)*0.5f,static_cast<float>(h)*0.5f};if(!app.inventoryOpen){d->AddLine({c.x-8.0f,c.y},{c.x+8.0f,c.y},imguiColor({0.95f,0.95f,0.88f}),1.5f);d->AddLine({c.x,c.y-8.0f},{c.x,c.y+8.0f},imguiColor({0.95f,0.95f,0.88f}),1.5f);}d->AddText({24.0f,static_cast<float>(h)-142.0f},imguiColor({0.98f,0.92f,0.84f}),"HP");drawImGuiBar(d,{70.0f,static_cast<float>(h)-140.0f},{240.0f,static_cast<float>(h)-121.0f},app.player.health,imguiColor({0.14f,0.030f,0.035f},0.86f),imguiColor({0.92f,0.16f,0.14f}));d->AddText({24.0f,static_cast<float>(h)-112.0f},imguiColor({0.98f,0.92f,0.84f}),"STA");drawImGuiBar(d,{70.0f,static_cast<float>(h)-110.0f},{240.0f,static_cast<float>(h)-91.0f},app.player.stamina,imguiColor({0.08f,0.08f,0.06f},0.86f),imguiColor({0.92f,0.74f,0.24f}));std::string dist="DRAW "+std::to_string(app.renderDistance);d->AddText({24.0f,static_cast<float>(h)-78.0f},imguiColor({0.70f,0.84f,0.98f}),dist.c_str());if(app.breakProgress>0.0f)drawImGuiBar(d,{static_cast<float>(w)*0.5f-130.0f,static_cast<float>(h)*0.5f+82.0f},{static_cast<float>(w)*0.5f+130.0f,static_cast<float>(h)*0.5f+100.0f},app.breakProgress,imguiColor({0.04f,0.045f,0.052f},0.88f),imguiColor({0.95f,0.72f,0.28f}));if(app.inventoryOpen){float invSlot=slot*0.95f;float invGap=invSlot*0.16f;float invTotal=invSlot*HotbarSlots+invGap*(HotbarSlots-1);ImVec2 panel{(static_cast<float>(w)-invTotal)*0.5f-30.0f,static_cast<float>(h)*0.16f};ImVec2 panelEnd{panel.x+invTotal+60.0f,panel.y+invSlot*3.0f+invGap*2.0f+70.0f};d->AddRectFilled({panel.x+8.0f,panel.y+10.0f},{panelEnd.x+8.0f,panelEnd.y+10.0f},imguiColor({0.01f,0.012f,0.016f},0.60f),14.0f);d->AddRectFilled(panel,panelEnd,imguiColor({0.050f,0.058f,0.070f},0.94f),14.0f);d->AddRect(panel,panelEnd,imguiColor({0.24f,0.30f,0.36f},0.95f),14.0f,0,1.6f);d->AddText({panel.x+24.0f,panel.y+18.0f},imguiColor({0.96f,0.88f,0.62f}),"Inventory");for(int row=0;row<3;row++){for(int col=0;col<HotbarSlots;col++){int idx=row*HotbarSlots+col;drawImGuiSlot(d,app.inventory[idx],{panel.x+30.0f+static_cast<float>(col)*(invSlot+invGap),panel.y+48.0f+static_cast<float>(row)*(invSlot+invGap)},invSlot,idx==app.selected);}}int hover=inventorySlotAt(app.mouseX,app.mouseY,w,h);if(hover>=0&&hover<InventorySlots&&hover!=app.dragSlot&&app.inventory[hover].count>0){const Slot& s=app.inventory[hover];ImVec2 m{static_cast<float>(app.mouseX),static_cast<float>(app.mouseY)};ImVec2 tp{std::min(m.x+18.0f,static_cast<float>(w)-230.0f),std::min(m.y+18.0f,static_cast<float>(h)-112.0f)};ImVec2 tq{tp.x+218.0f,tp.y+98.0f};d->AddRectFilled({tp.x+5.0f,tp.y+6.0f},{tq.x+5.0f,tq.y+6.0f},imguiColor({0.01f,0.012f,0.016f},0.65f),9.0f);d->AddRectFilled(tp,tq,imguiColor({0.050f,0.058f,0.070f},0.96f),9.0f);d->AddRect(tp,tq,imguiColor({0.30f,0.35f,0.40f}),9.0f);d->AddText({tp.x+14.0f,tp.y+12.0f},imguiColor({0.96f,0.88f,0.62f}),blockName(s.type));std::string cnt="Count: "+std::to_string(s.count);std::string hard="Hardness: "+std::to_string(static_cast<int>(blockHardness(s.type)*100.0f));d->AddText({tp.x+14.0f,tp.y+42.0f},imguiColor({0.82f,0.88f,0.94f}),cnt.c_str());d->AddText({tp.x+14.0f,tp.y+66.0f},imguiColor({0.82f,0.88f,0.94f}),hard.c_str());}if(app.dragSlot>=0&&app.dragSlot<InventorySlots&&app.inventory[app.dragSlot].count>0){ImVec2 m{static_cast<float>(app.mouseX),static_cast<float>(app.mouseY)};drawImGuiIcon(d,{m.x-invSlot*0.45f,m.y-invSlot*0.45f},{m.x+invSlot*0.45f,m.y+invSlot*0.45f},app.inventory[app.dragSlot].type);}}ImGui::End();}
static void drawMesh(const Mesh& mesh,GLint uModel,GLint uMvp,GLint uTint,Mat4 projection,Mat4 view,Mat4 modelMatrix,Vec3 tint){if(!mesh.indices)return;Mat4 mvp=mul(projection,mul(view,modelMatrix));oglUniformMatrix4fv(uModel,1,GL_FALSE,modelMatrix.m);oglUniformMatrix4fv(uMvp,1,GL_FALSE,mvp.m);oglUniform3f(uTint,tint.x,tint.y,tint.z);oglBindVertexArray(mesh.vao);glDrawElements(GL_TRIANGLES,mesh.indices,GL_UNSIGNED_INT,nullptr);}
static const char* worldVs=R"glsl(#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec3 aColor;
layout(location=3) in float aType;
uniform mat4 uModel;
uniform mat4 uMvp;
uniform mat4 uLightMvp;
uniform float uTime;
out vec3 vNormal;
out vec3 vColor;
out vec3 vPos;
out vec4 vLightPos;
out float vType;
void main(){
vec3 pos=aPos;
if(aType>5.5&&aType<6.5&&aNormal.y>0.5)pos.y+=sin(pos.x*0.65+pos.z*0.48+uTime*1.7)*0.050+sin(pos.x*1.35-pos.z*0.78+uTime*1.1)*0.024;
vec4 world=uModel*vec4(pos,1.0);
vPos=world.xyz;
vNormal=normalize(mat3(uModel)*aNormal);
vColor=aColor;
vLightPos=uLightMvp*world;
vType=aType;
gl_Position=uMvp*vec4(pos,1.0);
})glsl";
static const char* worldFs=R"glsl(#version 330 core
in vec3 vNormal;
in vec3 vColor;
in vec3 vPos;
in vec4 vLightPos;
in float vType;
uniform vec3 uTint;
uniform vec3 uSunDir;
uniform vec3 uSunColor;
uniform vec3 uSkyColor;
uniform vec3 uCamera;
uniform float uTime;
uniform sampler2D uShadowMap;
out vec4 FragColor;
float hash(vec3 p){
return fract(sin(dot(p,vec3(12.9898,78.233,37.719)))*43758.5453);
}
vec3 textureColor(vec3 color,float type,vec3 p,vec3 n){
float block=hash(floor(p));
float fine=hash(floor(p*13.0+n*5.0));
float line=hash(floor(p*29.0+n*9.0));
float grain=mix(0.93,1.07,block)*mix(0.96,1.04,fine);
if(type>0.5&&type<1.5)color=mix(color,vec3(0.22,0.62,0.18),n.y>0.5?0.46:0.12)*grain;
if(type>1.5&&type<2.5)color*=grain*mix(0.94,1.06,line);
if(type>2.5&&type<3.5)color*=mix(0.88,1.10,fine);
if(type>3.5&&type<4.5)color*=mix(0.97,1.08,block);
if(type>4.5&&type<5.5)color*=0.82+abs(sin((p.y+p.x*0.18+p.z*0.16)*5.5))*0.18;
if(type>5.5&&type<6.5){float wave=sin(p.x*1.6+uTime*1.5)+sin(p.z*2.1-uTime*1.2);color=mix(color,vec3(0.10,0.46,0.84),0.62+wave*0.025);}
if(type>6.5&&type<7.5)color=mix(color,vec3(0.14,0.46,0.12),0.26)*mix(0.92,1.08,fine);
if(type>7.5&&type<8.5)color=mix(color,vec3(0.74,0.65,0.38),0.34)*mix(0.94,1.06,fine);
return color;
}
float shadowFactor(vec3 n){
vec3 proj=vLightPos.xyz/vLightPos.w;
proj=proj*0.5+0.5;
if(proj.z<0.0||proj.z>1.0||proj.x<0.0||proj.x>1.0||proj.y<0.0||proj.y>1.0)return 1.0;
float bias=max(0.0018*(1.0-dot(n,normalize(uSunDir))),0.00055);
vec2 texel=1.0/textureSize(uShadowMap,0);
float sum=0.0;
for(int x=-2;x<=2;x++)for(int y=-2;y<=2;y++){
float d=texture(uShadowMap,proj.xy+vec2(x,y)*texel).r;
sum+=proj.z-bias>d?0.50:1.0;
}
return sum/25.0;
}
void main(){
vec3 n=normalize(vNormal);
vec3 viewDir=normalize(uCamera-vPos);
float diffuse=max(dot(n,normalize(uSunDir)),0.0);
float water=step(5.5,vType)*step(vType,6.5);
float shadow=mix(shadowFactor(n),1.0,water*0.72);
float wrap=diffuse*shadow*0.86+0.28;
float spec=pow(max(dot(reflect(-normalize(uSunDir),n),viewDir),0.0),42.0)*0.18*shadow;
float fog=clamp(length(vPos-uCamera)/170.0,0.0,1.0);
float sideShadow=mix(0.72,1.0,abs(n.y));
float contact=0.96+hash(floor(vPos))*0.04;
vec3 base=textureColor(vColor*uTint,vType,vPos,n);
vec3 color=base*wrap*sideShadow*contact+uSunColor*spec*(1.0-water);
float shimmer=sin(vPos.x*2.5+uTime*1.8)*sin(vPos.z*1.9-uTime*1.4);
color=mix(color,vec3(0.18,0.55,0.88),water*(0.34+shimmer*0.04));
FragColor=vec4(mix(color,uSkyColor,fog),mix(1.0,0.62,water));
})glsl";
static const char* shadowVs=R"glsl(#version 330 core
layout(location=0) in vec3 aPos;
uniform mat4 uLightMvp;
void main(){
gl_Position=uLightMvp*vec4(aPos,1.0);
})glsl";
static const char* shadowFs=R"glsl(#version 330 core
void main(){
}
)glsl";
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
uniform float uTime;
out vec4 FragColor;
float h2(vec2 p){
return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453123);
}
float n2(vec2 p){
vec2 i=floor(p);
vec2 f=fract(p);
f=f*f*(3.0-2.0*f);
float a=h2(i);
float b=h2(i+vec2(1.0,0.0));
float c=h2(i+vec2(0.0,1.0));
float d=h2(i+vec2(1.0,1.0));
return mix(mix(a,b,f.x),mix(c,d,f.x),f.y);
}
float fbm(vec2 p){
float v=0.0;
float a=0.5;
for(int i=0;i<5;i++){v+=n2(p)*a;p*=2.04;p+=vec2(11.7,4.3);a*=0.52;}
return v;
}
void main(){
vec3 dir=normalize(vDir);
float h=clamp(dir.y*0.5+0.5,0.0,1.0);
vec3 horizon=vec3(0.78,0.84,0.92);
vec3 zenith=vec3(0.18,0.42,0.82);
vec3 sunDir=normalize(uSunDir);
float sun=max(dot(dir,sunDir),0.0);
float disk=smoothstep(0.997,0.9995,sun);
float glow=pow(sun,32.0)*0.42+pow(sun,384.0)*0.72;
float cloudMask=smoothstep(0.03,0.42,dir.y);
vec2 cloudUv=dir.xz/max(dir.y+0.28,0.18)*0.46+vec2(uTime*0.006,uTime*0.002);
float cloud=fbm(cloudUv*1.4)*0.72+fbm(cloudUv*4.1+20.0)*0.28;
cloud=smoothstep(0.52,0.76,cloud)*cloudMask;
float cloudLight=clamp(dot(normalize(vec3(dir.x,0.25,dir.z)),sunDir)*0.5+0.5,0.0,1.0);
vec3 cloudColor=mix(vec3(0.72,0.76,0.80),vec3(1.0,0.94,0.82),cloudLight);
float rays=pow(sun,7.0)*0.34*(1.0-cloud*0.52)+pow(max(dot(dir,sunDir*vec3(1.0,0.55,1.0)),0.0),18.0)*0.10;
vec3 color=mix(horizon,zenith,h)+vec3(1.0,0.64,0.24)*glow+vec3(1.0,0.88,0.55)*disk;
color=mix(color,cloudColor,cloud*0.72);
color+=vec3(1.0,0.72,0.34)*rays;
FragColor=vec4(color,1.0);
})glsl";
int main(int argc,char** argv){try{bool smoke=argc>1&&std::string(argv[1])=="--smoke";if(!glfwInit())throw std::runtime_error("GLFW init failed");glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);if(smoke)glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
#if defined(__APPLE__)
glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GLFW_TRUE);
#endif
GLFWwindow* window=glfwCreateWindow(1280,720,"OpenGLWorld",nullptr,nullptr);if(!window)throw std::runtime_error("Window creation failed");glfwMakeContextCurrent(window);glfwSwapInterval(1);loadGl();App app;std::vector<Chunk> chunks;app.chunks=&chunks;app.player.p=findSpawn();ensureChunks(chunks,app.player.p,app.renderDistance);keepPlayerClear(app);app.camera.p=app.player.p+Vec3{0.0f,1.62f,0.0f};rebuildDirtyChunks(chunks);glfwSetWindowUserPointer(window,&app);glfwSetCursorPosCallback(window,cursorCallback);glfwSetScrollCallback(window,scrollCallback);glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);GLuint worldProgram=createProgram(worldVs,worldFs);GLuint shadowProgram=createProgram(shadowVs,shadowFs);GLuint skyProgram=createProgram(skyVs,skyFs);GLint uModel=oglGetUniformLocation(worldProgram,"uModel");GLint uMvp=oglGetUniformLocation(worldProgram,"uMvp");GLint uTint=oglGetUniformLocation(worldProgram,"uTint");GLint uSunDir=oglGetUniformLocation(worldProgram,"uSunDir");GLint uSunColor=oglGetUniformLocation(worldProgram,"uSunColor");GLint uSkyColor=oglGetUniformLocation(worldProgram,"uSkyColor");GLint uCamera=oglGetUniformLocation(worldProgram,"uCamera");GLint uTime=oglGetUniformLocation(worldProgram,"uTime");GLint uLightMvp=oglGetUniformLocation(worldProgram,"uLightMvp");GLint uShadowMap=oglGetUniformLocation(worldProgram,"uShadowMap");GLint shLightMvp=oglGetUniformLocation(shadowProgram,"uLightMvp");GLint sForward=oglGetUniformLocation(skyProgram,"uForward");GLint sRight=oglGetUniformLocation(skyProgram,"uRight");GLint sUp=oglGetUniformLocation(skyProgram,"uUp");GLint sSunDir=oglGetUniformLocation(skyProgram,"uSunDir");GLint sTime=oglGetUniformLocation(skyProgram,"uTime");Mesh skyQuad=createScreenQuad();ShadowMap shadow=createShadowMap(2048);if(smoke){clearChunks(chunks);glfwTerminate();return 0;}IMGUI_CHECKVERSION();ImGui::CreateContext();ImGuiIO& io=ImGui::GetIO();io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard;ImGui::StyleColorsDark();ImGuiStyle& style=ImGui::GetStyle();style.WindowRounding=12.0f;style.FrameRounding=8.0f;style.PopupRounding=10.0f;style.WindowBorderSize=1.0f;style.FrameBorderSize=0.0f;style.Colors[ImGuiCol_WindowBg]=ImVec4(0.035f,0.042f,0.052f,0.92f);style.Colors[ImGuiCol_Border]=ImVec4(0.22f,0.28f,0.34f,0.86f);style.Colors[ImGuiCol_Text]=ImVec4(0.92f,0.91f,0.86f,1.0f);ImGui_ImplGlfw_InitForOpenGL(window,false);ImGui_ImplOpenGL3_Init("#version 330 core");Vec3 sunDir=norm({-0.38f,0.78f,-0.48f});Vec3 sunColor{1.0f,0.88f,0.62f};Vec3 skyColor{0.62f,0.76f,0.92f};double last=glfwGetTime();while(!glfwWindowShouldClose(window)){double now=glfwGetTime();float dt=std::min(static_cast<float>(now-last),0.033f);last=now;ensureChunks(chunks,app.player.p,app.renderDistance);glfwPollEvents();ImGui_ImplOpenGL3_NewFrame();ImGui_ImplGlfw_NewFrame();ImGui::NewFrame();updateInput(window,app,dt);ensureChunks(chunks,app.player.p,app.renderDistance);rebuildDirtyChunks(chunks,3);int w=0;int h=0;glfwGetFramebufferSize(window,&w,&h);float aspect=static_cast<float>(std::max(w,1))/static_cast<float>(std::max(h,1));Vec3 forward=cameraForward(app.camera);Vec3 right=norm(cross(forward,{0.0f,1.0f,0.0f}));Vec3 up=norm(cross(right,forward));Mat4 proj=perspective(65.0f*Pi/180.0f,aspect,0.1f,420.0f);Mat4 view=lookAt(app.camera.p,app.camera.p+forward,{0.0f,1.0f,0.0f});Mat4 lightView=lookAt(app.camera.p+sunDir*118.0f,app.camera.p,{0.0f,1.0f,0.0f});Mat4 lightProj=ortho(-112.0f,112.0f,-112.0f,112.0f,1.0f,250.0f);Mat4 lightMvp=mul(lightProj,lightView);oglBindFramebuffer(GL_FRAMEBUFFER,shadow.fbo);glViewport(0,0,shadow.size,shadow.size);glClear(GL_DEPTH_BUFFER_BIT);glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);glDisable(GL_BLEND);glDisable(GL_CULL_FACE);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);oglUseProgram(shadowProgram);oglUniformMatrix4fv(shLightMvp,1,GL_FALSE,lightMvp.m);for(const Chunk& c:chunks){if(c.mesh.indices){oglBindVertexArray(c.mesh.vao);glDrawElements(GL_TRIANGLES,c.mesh.indices,GL_UNSIGNED_INT,nullptr);}}oglBindFramebuffer(GL_FRAMEBUFFER,0);glViewport(0,0,w,h);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);oglUseProgram(skyProgram);oglUniform3f(sForward,forward.x,forward.y,forward.z);oglUniform3f(sRight,right.x*aspect*0.72f,right.y*aspect*0.72f,right.z*aspect*0.72f);oglUniform3f(sUp,up.x*0.72f,up.y*0.72f,up.z*0.72f);oglUniform3f(sSunDir,sunDir.x,sunDir.y,sunDir.z);oglUniform1f(sTime,static_cast<float>(now));oglBindVertexArray(skyQuad.vao);glDrawElements(GL_TRIANGLES,skyQuad.indices,GL_UNSIGNED_INT,nullptr);glEnable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glPolygonMode(GL_FRONT_AND_BACK,app.wireframe?GL_LINE:GL_FILL);oglUseProgram(worldProgram);oglUniform3f(uSunDir,sunDir.x,sunDir.y,sunDir.z);oglUniform3f(uSunColor,sunColor.x,sunColor.y,sunColor.z);oglUniform3f(uSkyColor,skyColor.x,skyColor.y,skyColor.z);oglUniform3f(uCamera,app.camera.p.x,app.camera.p.y,app.camera.p.z);oglUniform1f(uTime,static_cast<float>(now));oglUniformMatrix4fv(uLightMvp,1,GL_FALSE,lightMvp.m);oglActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,shadow.texture);oglUniform1i(uShadowMap,0);for(const Chunk& c:chunks)drawMesh(c.mesh,uModel,uMvp,uTint,proj,view,identity(),{1.0f,1.0f,1.0f});glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDepthMask(GL_FALSE);for(const Chunk& c:chunks)drawMesh(c.waterMesh,uModel,uMvp,uTint,proj,view,identity(),{1.0f,1.0f,1.0f});glDepthMask(GL_TRUE);glDisable(GL_BLEND);drawImGuiHud(app,w,h);ImGui::Render();ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());glfwSwapBuffers(window);}ImGui_ImplOpenGL3_Shutdown();ImGui_ImplGlfw_Shutdown();ImGui::DestroyContext();destroyMesh(skyQuad);clearChunks(chunks);glfwTerminate();return 0;}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;glfwTerminate();return 1;}}
