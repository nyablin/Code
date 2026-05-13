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
constexpr int BlockTypes=14;
struct Vec3{float x;float y;float z;};
struct Mat4{float m[16];};
struct Vertex{Vec3 p;Vec3 n;Vec3 c;float t;};
struct Mesh{GLuint vao=0;GLuint vbo=0;GLuint ebo=0;GLsizei indices=0;};
struct ShadowMap{GLuint fbo=0;GLuint texture=0;int size=1024;};
struct Chunk{int cx=0;int cz=0;std::vector<unsigned char> blocks;Mesh mesh;Mesh waterMesh;bool dirty=true;};
struct RayHit{bool hit=false;int x=0;int y=0;int z=0;int px=0;int py=0;int pz=0;unsigned char type=0;};
struct Slot{unsigned char type=0;int count=0;float wear=1.0f;};
struct RainParticle{Vec3 p{0.0f,0.0f,0.0f};float speed=18.0f;float drift=0.0f;};
struct Puddle{Vec3 p{0.0f,0.0f,0.0f};float radius=0.18f;float life=8.0f;float strength=0.0f;};
struct Camera{Vec3 p{0.0f,22.0f,0.0f};float yaw=-90.0f;float pitch=-18.0f;double lastX=640.0;double lastY=360.0;bool firstMouse=true;};
struct Player{Vec3 p{0.0f,24.0f,0.0f};Vec3 v{0.0f,0.0f,0.0f};float health=1.0f;float stamina=1.0f;float oxygen=1.0f;bool grounded=false;bool swimming=false;bool crouching=false;};
struct App{Camera camera;Player player;bool wireframe=false;bool lastWire=false;bool lastReset=false;bool lastPlace=false;bool lastSort=false;bool lastTab=false;bool lastDebug=false;bool lastRain=false;bool lastDistUp=false;bool lastDistDown=false;bool lastView=false;bool inventoryOpen=false;bool debugOpen=false;bool speedMode=false;bool creative=false;bool fly=false;bool noclip=false;bool waterEffect=true;bool clouds=true;bool dayCycle=true;bool rain=false;bool thirdPerson=false;bool avatarAnim=true;bool playerSheltered=false;bool lastMouseLeft=false;bool lastInvRight=false;int selected=0;int renderDistance=ChunkRadius;float fogDistance=170.0f;float dayTime=0.28f;float bobPhase=0.0f;int dragSlot=-1;int mouseDownSlot=-1;float mouseHold=0.0f;double mouseX=640.0;double mouseY=360.0;std::vector<RainParticle> rainParticles;std::vector<Puddle> puddles;float tailYaw[7]{};float tailYawVel[7]{};float tailPitch[7]{};float tailPitchVel[7]{};Slot inventory[InventorySlots]{{1,24,1.0f},{2,24,1.0f},{3,18,1.0f},{4,8,1.0f},{5,12,1.0f},{6,8,1.0f},{7,16,1.0f},{8,18,1.0f},{13,12,1.0f},{10,6,1.0f},{11,4,1.0f},{12,2,1.0f},{0,0,1.0f}};bool lastSlot[HotbarSlots]{};int breakX=0;int breakY=0;int breakZ=0;float breakTimer=0.0f;float breakProgress=0.0f;std::vector<Chunk>* chunks=nullptr;};
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
static bool isPlant(unsigned char type){return type==9;}
static bool isSolid(unsigned char type){return type!=0&&!isWater(type)&&!isPlant(type);}
static Vec3 itemColor(unsigned char type){Vec3 colors[BlockTypes]{{0.04f,0.05f,0.06f},{0.30f,0.58f,0.20f},{0.45f,0.31f,0.18f},{0.43f,0.43f,0.42f},{0.86f,0.88f,0.84f},{0.47f,0.30f,0.14f},{0.20f,0.52f,0.85f},{0.25f,0.55f,0.24f},{0.72f,0.64f,0.39f},{0.30f,0.72f,0.18f},{0.18f,0.18f,0.17f},{0.58f,0.45f,0.34f},{0.86f,0.66f,0.22f},{0.62f,0.40f,0.18f}};return colors[std::clamp(static_cast<int>(type),0,BlockTypes-1)];}
static Vec3 blockColor(unsigned char type,int y){Vec3 c=itemColor(type);float shade=0.82f+fract(static_cast<float>(y)*0.137f)*0.18f;return c*shade;}
static float blockHardness(unsigned char type){float h[BlockTypes]{0.0f,0.55f,0.65f,1.45f,0.45f,0.95f,0.35f,0.30f,0.50f,0.18f,1.65f,1.95f,2.25f,0.75f};return h[std::clamp(static_cast<int>(type),0,BlockTypes-1)];}
static Mat4 identity(){Mat4 r{};r.m[0]=1.0f;r.m[5]=1.0f;r.m[10]=1.0f;r.m[15]=1.0f;return r;}
static Mat4 mul(Mat4 a,Mat4 b){Mat4 r{};for(int c=0;c<4;c++){for(int row=0;row<4;row++){r.m[c*4+row]=a.m[0*4+row]*b.m[c*4+0]+a.m[1*4+row]*b.m[c*4+1]+a.m[2*4+row]*b.m[c*4+2]+a.m[3*4+row]*b.m[c*4+3];}}return r;}
static Mat4 perspective(float fov,float aspect,float nearPlane,float farPlane){Mat4 r{};float t=std::tan(fov*0.5f);r.m[0]=1.0f/(aspect*t);r.m[5]=1.0f/t;r.m[10]=-(farPlane+nearPlane)/(farPlane-nearPlane);r.m[11]=-1.0f;r.m[14]=-(2.0f*farPlane*nearPlane)/(farPlane-nearPlane);return r;}
static Mat4 ortho(float l,float r,float b,float t,float n,float f){Mat4 m{};m.m[0]=2.0f/(r-l);m.m[5]=2.0f/(t-b);m.m[10]=-2.0f/(f-n);m.m[12]=-(r+l)/(r-l);m.m[13]=-(t+b)/(t-b);m.m[14]=-(f+n)/(f-n);m.m[15]=1.0f;return m;}
static Mat4 lookAt(Vec3 eye,Vec3 center,Vec3 up){Vec3 f=norm(center-eye);Vec3 s=norm(cross(f,up));Vec3 u=cross(s,f);Mat4 r=identity();r.m[0]=s.x;r.m[4]=s.y;r.m[8]=s.z;r.m[1]=u.x;r.m[5]=u.y;r.m[9]=u.z;r.m[2]=-f.x;r.m[6]=-f.y;r.m[10]=-f.z;r.m[12]=-dot(s,eye);r.m[13]=-dot(u,eye);r.m[14]=dot(f,eye);return r;}
static Mat4 translate(Vec3 v){Mat4 r=identity();r.m[12]=v.x;r.m[13]=v.y;r.m[14]=v.z;return r;}
static Mat4 scaleM(Vec3 v){Mat4 r=identity();r.m[0]=v.x;r.m[5]=v.y;r.m[10]=v.z;return r;}
static Mat4 rotateX(float a){Mat4 r=identity();float c=std::cos(a);float s=std::sin(a);r.m[5]=c;r.m[6]=s;r.m[9]=-s;r.m[10]=c;return r;}
static Mat4 rotateY(float a){Mat4 r=identity();float c=std::cos(a);float s=std::sin(a);r.m[0]=c;r.m[2]=-s;r.m[8]=s;r.m[10]=c;return r;}
static Mat4 rotateZ(float a){Mat4 r=identity();float c=std::cos(a);float s=std::sin(a);r.m[0]=c;r.m[1]=s;r.m[4]=-s;r.m[5]=c;return r;}
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
static int biomeAt(int x,int z){float fx=static_cast<float>(x);float fz=static_cast<float>(z);float temp=noise(fx*0.006f+240.0f,fz*0.006f-160.0f);float wet=noise(fx*0.007f-420.0f,fz*0.007f+310.0f);float ridge=1.0f-std::abs(noise(fx*0.012f+80.0f,fz*0.012f-95.0f));if(temp>0.30f&&wet<-0.20f)return 2;if(temp<-0.34f||ridge>0.72f)return 3;if(wet>0.30f&&temp>-0.18f)return 1;if(ridge>0.56f)return 4;return 0;}
static const char* biomeName(int biome){static const char* names[]{"PLAINS","WET FOREST","DESERT","SNOW","HIGHLANDS"};return names[std::clamp(biome,0,4)];}
static int blockHeight(int x,int z){float fx=static_cast<float>(x);float fz=static_cast<float>(z);int biome=biomeAt(x,z);float broad=noise(fx*0.010f-70.0f,fz*0.010f+20.0f);float hills=noise(fx*0.032f+30.0f,fz*0.032f-12.0f);float detail=noise(fx*0.078f,fz*0.078f);float ridge=1.0f-std::abs(noise(fx*0.020f+90.0f,fz*0.020f-110.0f));float h=22.0f+broad*17.0f+hills*9.0f+detail*2.6f+ridge*ridge*8.0f;if(biome==2)h=18.0f+broad*6.0f+hills*2.4f+detail*1.2f;if(biome==1)h+=2.0f+detail*1.6f;if(biome==3)h+=7.0f+ridge*ridge*7.5f;if(biome==4)h+=10.0f+hills*6.0f+ridge*ridge*11.0f;return std::clamp(static_cast<int>(std::floor(h)),5,WorldHeight-8);}
static Chunk* findChunk(std::vector<Chunk>& chunks,int cx,int cz){for(Chunk& c:chunks)if(c.cx==cx&&c.cz==cz)return &c;return nullptr;}
static const Chunk* findChunkConst(const std::vector<Chunk>& chunks,int cx,int cz){for(const Chunk& c:chunks)if(c.cx==cx&&c.cz==cz)return &c;return nullptr;}
static unsigned char getBlock(const std::vector<Chunk>& chunks,int wx,int y,int wz){if(y<0||y>=WorldHeight)return 0;int cx=divFloor(wx,ChunkSize);int cz=divFloor(wz,ChunkSize);int lx=modFloor(wx,ChunkSize);int lz=modFloor(wz,ChunkSize);const Chunk* c=findChunkConst(chunks,cx,cz);return c?c->blocks[blockIndex(lx,y,lz)]:0;}
static void markChunkDirty(std::vector<Chunk>& chunks,int cx,int cz){if(Chunk* c=findChunk(chunks,cx,cz))c->dirty=true;}
static void markBlockDirty(std::vector<Chunk>& chunks,int wx,int wz){int cx=divFloor(wx,ChunkSize);int cz=divFloor(wz,ChunkSize);int lx=modFloor(wx,ChunkSize);int lz=modFloor(wz,ChunkSize);markChunkDirty(chunks,cx,cz);if(lx==0)markChunkDirty(chunks,cx-1,cz);if(lx==ChunkSize-1)markChunkDirty(chunks,cx+1,cz);if(lz==0)markChunkDirty(chunks,cx,cz-1);if(lz==ChunkSize-1)markChunkDirty(chunks,cx,cz+1);}
static void setBlock(std::vector<Chunk>& chunks,int wx,int y,int wz,unsigned char type){if(y<0||y>=WorldHeight)return;int cx=divFloor(wx,ChunkSize);int cz=divFloor(wz,ChunkSize);int lx=modFloor(wx,ChunkSize);int lz=modFloor(wz,ChunkSize);Chunk* c=findChunk(chunks,cx,cz);if(c){c->blocks[blockIndex(lx,y,lz)]=type;markBlockDirty(chunks,wx,wz);}}
static bool treeAt(int x,int z){int h=blockHeight(x,z);int biome=biomeAt(x,z);float dry=noise(static_cast<float>(x)*0.020f+140.0f,static_cast<float>(z)*0.020f-40.0f);float chance=biome==1?0.910f:(biome==0?0.956f:(biome==3?0.974f:0.966f));float minDry=biome==1?-0.72f:-0.54f;return biome!=2&&h>WaterLevel+2&&h<WorldHeight-10&&dry>minDry&&rand2(x*31,z*37)>chance;}
static bool caveAt(int x,int y,int z,int h){if(y<5||y>h-5)return false;float c=noise(static_cast<float>(x)*0.045f+static_cast<float>(y)*0.026f,static_cast<float>(z)*0.045f-static_cast<float>(y)*0.019f)*0.70f+noise(static_cast<float>(x)*0.095f-70.0f,static_cast<float>(z)*0.095f+static_cast<float>(y)*0.041f)*0.30f;return c>0.62f;}
static unsigned char oreType(int x,int y,int z){float r=rand2(x*71+y*17,z*79-y*11);if(y<18&&r>0.974f)return 12;if(y<32&&r>0.956f)return 11;if(y<46&&r>0.926f)return 10;return 3;}
static void setLocalBlock(Chunk& c,int wx,int y,int wz,unsigned char type){int lx=wx-c.cx*ChunkSize;int lz=wz-c.cz*ChunkSize;if(lx>=0&&lx<ChunkSize&&lz>=0&&lz<ChunkSize&&y>=0&&y<WorldHeight)c.blocks[blockIndex(lx,y,lz)]=type;}
static Chunk makeChunk(int cx,int cz){Chunk c;c.cx=cx;c.cz=cz;c.blocks.assign(static_cast<size_t>(ChunkSize*WorldHeight*ChunkSize),0);for(int z=0;z<ChunkSize;z++){for(int x=0;x<ChunkSize;x++){int wx=cx*ChunkSize+x;int wz=cz*ChunkSize+z;int h=blockHeight(wx,wz);int biome=biomeAt(wx,wz);float dry=noise(static_cast<float>(wx)*0.020f+140.0f,static_cast<float>(wz)*0.020f-40.0f);bool desert=biome==2;bool snow=biome==3||h>44;bool wet=biome==1&&dry<0.28f;bool beach=h<=WaterLevel+1||dry<-0.52f||(wet&&h<=WaterLevel+3);for(int y=0;y<=h;y++){if(caveAt(wx,y,wz,h)){if(y<WaterLevel-1)c.blocks[blockIndex(x,y,z)]=6;continue;}unsigned char type=3;if(y==h)type=(beach||desert)?8:(snow?4:1);else if(y>h-4)type=(beach||desert)?8:2;else type=oreType(wx,y,wz);c.blocks[blockIndex(x,y,z)]=type;}if(h<WaterLevel){for(int y=h+1;y<=WaterLevel;y++)c.blocks[blockIndex(x,y,z)]=6;}float grassChance=biome==1?0.03f:(biome==0?0.24f:(biome==4?0.48f:0.82f));if(h>WaterLevel+2&&h<WorldHeight-2&&c.blocks[blockIndex(x,h,z)]==1&&rand2(wx*13,wz*23)>grassChance)c.blocks[blockIndex(x,h+1,z)]=9;if(biome==4&&h>WaterLevel+4&&h<WorldHeight-2&&rand2(wx*53,wz*57)>0.970f)c.blocks[blockIndex(x,h+1,z)]=3;if((biome==2||biome==3||biome==4)&&h>WaterLevel+2&&h<WorldHeight-3&&rand2(wx*61,wz*67)>0.986f){c.blocks[blockIndex(x,h+1,z)]=3;if(rand2(wx*71,wz*73)>0.35f)c.blocks[blockIndex(x,h+2,z)]=3;}if(biome==1&&h>WaterLevel&&h<WorldHeight-2&&rand2(wx*83,wz*89)>0.955f)c.blocks[blockIndex(x,h+1,z)]=9;if(biome==2&&h>WaterLevel+2&&h<WorldHeight-2&&rand2(wx*97,wz*101)>0.992f)c.blocks[blockIndex(x,h+1,z)]=5;}}for(int z=-4;z<ChunkSize+4;z++){for(int x=-4;x<ChunkSize+4;x++){int wx=cx*ChunkSize+x;int wz=cz*ChunkSize+z;if(!treeAt(wx,wz))continue;int h=blockHeight(wx,wz);int biome=biomeAt(wx,wz);int trunk=4+static_cast<int>((rand2(wx*11,wz*17)+1.0f)*1.5f)+(biome==1?1:0);for(int y=h+1;y<=h+trunk;y++)setLocalBlock(c,wx,y,wz,5);int topExtra=biome==1?1:0;for(int dy=-2;dy<=3+topExtra;dy++){int radius=dy>1?1:2+(biome==1&&dy<2?1:0);for(int dz=-radius;dz<=radius;dz++){for(int dx=-radius;dx<=radius;dx++){int dist=std::abs(dx)+std::abs(dz);if(dx==0&&dz==0&&dy<=0)continue;if(dist>radius+1)continue;if(dist==radius+1&&rand2((wx+dx)*43+dy,(wz+dz)*47)<-0.15f)continue;setLocalBlock(c,wx+dx,h+trunk+dy,wz+dz,7);}}}}}c.dirty=true;return c;}
static void addFace(std::vector<Vertex>& vertices,std::vector<unsigned int>& indices,float x,float y,float z,int face,Vec3 color,unsigned char type){static const Vec3 normals[6]{{1.0f,0.0f,0.0f},{-1.0f,0.0f,0.0f},{0.0f,1.0f,0.0f},{0.0f,-1.0f,0.0f},{0.0f,0.0f,1.0f},{0.0f,0.0f,-1.0f}};static const Vec3 p[6][4]{{{1,0,0},{1,1,0},{1,1,1},{1,0,1}},{{0,0,1},{0,1,1},{0,1,0},{0,0,0}},{{0,1,1},{1,1,1},{1,1,0},{0,1,0}},{{0,0,0},{1,0,0},{1,0,1},{0,0,1}},{{1,0,1},{1,1,1},{0,1,1},{0,0,1}},{{0,0,0},{0,1,0},{1,1,0},{1,0,0}}};unsigned int base=static_cast<unsigned int>(vertices.size());float light[6]{0.82f,0.68f,1.0f,0.50f,0.76f,0.62f};Vec3 c=color*light[face];for(int i=0;i<4;i++)vertices.push_back({{x+p[face][i].x,y+p[face][i].y,z+p[face][i].z},normals[face],c,static_cast<float>(type)});indices.push_back(base);indices.push_back(base+1);indices.push_back(base+2);indices.push_back(base);indices.push_back(base+2);indices.push_back(base+3);}
static Mesh createAvatarCube(){std::vector<Vertex> vertices;std::vector<unsigned int> indices;static const Vec3 normals[6]{{1.0f,0.0f,0.0f},{-1.0f,0.0f,0.0f},{0.0f,1.0f,0.0f},{0.0f,-1.0f,0.0f},{0.0f,0.0f,1.0f},{0.0f,0.0f,-1.0f}};static const Vec3 p[6][4]{{{0.5f,-0.5f,-0.5f},{0.5f,0.5f,-0.5f},{0.5f,0.5f,0.5f},{0.5f,-0.5f,0.5f}},{{-0.5f,-0.5f,0.5f},{-0.5f,0.5f,0.5f},{-0.5f,0.5f,-0.5f},{-0.5f,-0.5f,-0.5f}},{{-0.5f,0.5f,0.5f},{0.5f,0.5f,0.5f},{0.5f,0.5f,-0.5f},{-0.5f,0.5f,-0.5f}},{{-0.5f,-0.5f,-0.5f},{0.5f,-0.5f,-0.5f},{0.5f,-0.5f,0.5f},{-0.5f,-0.5f,0.5f}},{{0.5f,-0.5f,0.5f},{0.5f,0.5f,0.5f},{-0.5f,0.5f,0.5f},{-0.5f,-0.5f,0.5f}},{{-0.5f,-0.5f,-0.5f},{-0.5f,0.5f,-0.5f},{0.5f,0.5f,-0.5f},{0.5f,-0.5f,-0.5f}}};for(int f=0;f<6;f++){unsigned int base=static_cast<unsigned int>(vertices.size());for(int i=0;i<4;i++)vertices.push_back({p[f][i],normals[f],{1.0f,1.0f,1.0f},0.0f});indices.push_back(base);indices.push_back(base+1);indices.push_back(base+2);indices.push_back(base);indices.push_back(base+2);indices.push_back(base+3);}return uploadMesh(vertices,indices);}
static void addPlant(std::vector<Vertex>& vertices,std::vector<unsigned int>& indices,float x,float y,float z,Vec3 color,unsigned char type){unsigned int base=static_cast<unsigned int>(vertices.size());Vec3 c=color*1.08f;Vec3 n{0.0f,1.0f,0.0f};vertices.push_back({{x+0.15f,y,z+0.15f},n,c,static_cast<float>(type)});vertices.push_back({{x+0.15f,y+0.88f,z+0.15f},n,c,static_cast<float>(type)});vertices.push_back({{x+0.85f,y+0.88f,z+0.85f},n,c,static_cast<float>(type)});vertices.push_back({{x+0.85f,y,z+0.85f},n,c,static_cast<float>(type)});vertices.push_back({{x+0.85f,y,z+0.15f},n,c,static_cast<float>(type)});vertices.push_back({{x+0.85f,y+0.88f,z+0.15f},n,c,static_cast<float>(type)});vertices.push_back({{x+0.15f,y+0.88f,z+0.85f},n,c,static_cast<float>(type)});vertices.push_back({{x+0.15f,y,z+0.85f},n,c,static_cast<float>(type)});unsigned int ids[12]{0,1,2,0,2,3,4,5,6,4,6,7};for(unsigned int id:ids)indices.push_back(base+id);}
static void rebuildChunkMesh(std::vector<Chunk>& chunks,Chunk& chunk){std::vector<Vertex> solidVertices;std::vector<unsigned int> solidIndices;std::vector<Vertex> waterVertices;std::vector<unsigned int> waterIndices;solidVertices.reserve(ChunkSize*ChunkSize*256);waterVertices.reserve(ChunkSize*ChunkSize*32);int dirs[6][3]{{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};for(int y=0;y<WorldHeight;y++){for(int z=0;z<ChunkSize;z++){for(int x=0;x<ChunkSize;x++){unsigned char type=chunk.blocks[blockIndex(x,y,z)];if(!type)continue;int wx=chunk.cx*ChunkSize+x;int wz=chunk.cz*ChunkSize+z;if(isPlant(type)){addPlant(solidVertices,solidIndices,static_cast<float>(wx),static_cast<float>(y),static_cast<float>(wz),blockColor(type,y),type);continue;}if(isWater(type)){for(int f=0;f<6;f++){unsigned char n=getBlock(chunks,wx+dirs[f][0],y+dirs[f][1],wz+dirs[f][2]);if(n!=6)addFace(waterVertices,waterIndices,static_cast<float>(wx),static_cast<float>(y),static_cast<float>(wz),f,blockColor(type,y),type);}}else{for(int f=0;f<6;f++){unsigned char n=getBlock(chunks,wx+dirs[f][0],y+dirs[f][1],wz+dirs[f][2]);if(!isSolid(n))addFace(solidVertices,solidIndices,static_cast<float>(wx),static_cast<float>(y),static_cast<float>(wz),f,blockColor(type,y),type);}}}}}destroyMesh(chunk.mesh);destroyMesh(chunk.waterMesh);chunk.mesh=uploadMesh(solidVertices,solidIndices);chunk.waterMesh=uploadMesh(waterVertices,waterIndices);chunk.dirty=false;}
static void rebuildDirtyChunks(std::vector<Chunk>& chunks,int maxCount=9999){int done=0;for(size_t i=0;i<chunks.size()&&done<maxCount;i++){if(chunks[i].dirty){rebuildChunkMesh(chunks,chunks[i]);done++;}}}
static void clearChunks(std::vector<Chunk>& chunks){for(Chunk& c:chunks){destroyMesh(c.mesh);destroyMesh(c.waterMesh);}chunks.clear();}
static void ensureChunks(std::vector<Chunk>& chunks,Vec3 p,int radius=ChunkRadius){int bx=static_cast<int>(std::floor(p.x));int bz=static_cast<int>(std::floor(p.z));int pcx=divFloor(bx,ChunkSize);int pcz=divFloor(bz,ChunkSize);for(size_t i=0;i<chunks.size();){if(std::abs(chunks[i].cx-pcx)>radius+1||std::abs(chunks[i].cz-pcz)>radius+1){int cx=chunks[i].cx;int cz=chunks[i].cz;destroyMesh(chunks[i].mesh);destroyMesh(chunks[i].waterMesh);chunks.erase(chunks.begin()+static_cast<std::ptrdiff_t>(i));markChunkDirty(chunks,cx-1,cz);markChunkDirty(chunks,cx+1,cz);markChunkDirty(chunks,cx,cz-1);markChunkDirty(chunks,cx,cz+1);}else i++;}for(int z=pcz-radius;z<=pcz+radius;z++){for(int x=pcx-radius;x<=pcx+radius;x++){if(!findChunk(chunks,x,z)){chunks.push_back(makeChunk(x,z));markChunkDirty(chunks,x-1,z);markChunkDirty(chunks,x+1,z);markChunkDirty(chunks,x,z-1);markChunkDirty(chunks,x,z+1);}}}}
static Vec3 findSpawn(){for(int r=0;r<=128;r++){for(int z=-r;z<=r;z++){for(int x=-r;x<=r;x++){if(std::max(std::abs(x),std::abs(z))!=r)continue;int h=blockHeight(x,z);if(h<=WaterLevel+2)continue;bool clear=true;for(int dz=-3;dz<=3;dz++)for(int dx=-3;dx<=3;dx++)if(treeAt(x+dx,z+dz))clear=false;if(clear)return {static_cast<float>(x)+0.5f,static_cast<float>(h)+1.06f,static_cast<float>(z)+0.5f};}}}int h=std::max(blockHeight(0,0),WaterLevel+3);return {0.5f,static_cast<float>(h)+1.06f,0.5f};}
static bool playerHitsHeight(const std::vector<Chunk>& chunks,Vec3 p,float h){float hw=0.32f;int minX=static_cast<int>(std::floor(p.x-hw));int maxX=static_cast<int>(std::floor(p.x+hw));int minY=static_cast<int>(std::floor(p.y+0.03f));int maxY=static_cast<int>(std::floor(p.y+h-0.03f));int minZ=static_cast<int>(std::floor(p.z-hw));int maxZ=static_cast<int>(std::floor(p.z+hw));for(int y=minY;y<=maxY;y++)for(int z=minZ;z<=maxZ;z++)for(int x=minX;x<=maxX;x++)if(isSolid(getBlock(chunks,x,y,z)))return true;return false;}
static bool playerHits(const std::vector<Chunk>& chunks,Vec3 p){return playerHitsHeight(chunks,p,1.78f);}
static bool blockTouchesPlayer(Vec3 p,int x,int y,int z){float hw=0.38f;float h=1.86f;return p.x+hw>static_cast<float>(x)&&p.x-hw<static_cast<float>(x+1)&&p.y+h>static_cast<float>(y)&&p.y<static_cast<float>(y+1)&&p.z+hw>static_cast<float>(z)&&p.z-hw<static_cast<float>(z+1);}
static bool hasFooting(const std::vector<Chunk>& chunks,Vec3 p){int x=static_cast<int>(std::floor(p.x));int y=static_cast<int>(std::floor(p.y-0.08f));int z=static_cast<int>(std::floor(p.z));return isSolid(getBlock(chunks,x,y,z));}
static void keepPlayerClear(App& app){if(!app.chunks)return;for(int i=0;i<24&&playerHits(*app.chunks,app.player.p);i++)app.player.p.y+=0.25f;}
static RayHit raycast(const std::vector<Chunk>& chunks,Vec3 origin,Vec3 dir){RayHit hit;int lastX=static_cast<int>(std::floor(origin.x));int lastY=static_cast<int>(std::floor(origin.y));int lastZ=static_cast<int>(std::floor(origin.z));for(float t=0.0f;t<6.5f;t+=0.05f){Vec3 p=origin+dir*t;int x=static_cast<int>(std::floor(p.x));int y=static_cast<int>(std::floor(p.y));int z=static_cast<int>(std::floor(p.z));unsigned char type=getBlock(chunks,x,y,z);if(isSolid(type)||isPlant(type)){hit.hit=true;hit.x=x;hit.y=y;hit.z=z;hit.px=lastX;hit.py=lastY;hit.pz=lastZ;hit.type=type;return hit;}lastX=x;lastY=y;lastZ=z;}return hit;}
static void resetInventory(App& app){for(Slot& s:app.inventory)s={0,0,1.0f};app.inventory[0]={1,24,1.0f};app.inventory[1]={2,24,1.0f};app.inventory[2]={3,18,1.0f};app.inventory[3]={4,8,1.0f};app.inventory[4]={5,12,1.0f};app.inventory[5]={6,8,1.0f};app.inventory[6]={7,16,1.0f};app.inventory[7]={8,18,1.0f};app.inventory[8]={13,12,1.0f};app.inventory[9]={10,6,1.0f};app.inventory[10]={11,4,1.0f};app.inventory[11]={12,2,1.0f};app.selected=0;}
static bool addItem(App& app,unsigned char type,int count){for(Slot& s:app.inventory){if(s.type==type&&s.count>0&&s.count<64){int take=std::min(count,64-s.count);s.count+=take;count-=take;if(count<=0)return true;}}for(Slot& s:app.inventory){if(!s.count){int take=std::min(count,64);s={type,take,1.0f};count-=take;if(count<=0)return true;}}return count<=0;}
static int itemCount(const App& app,unsigned char type){int count=0;for(const Slot& s:app.inventory)if(s.type==type)count+=s.count;return count;}
static bool takeItem(App& app,unsigned char type,int count){if(itemCount(app,type)<count)return false;for(Slot& s:app.inventory){if(s.type!=type)continue;int take=std::min(count,s.count);s.count-=take;count-=take;if(s.count<=0)s={0,0,1.0f};if(count<=0)return true;}return count<=0;}
static bool canCraft(const App& app,unsigned char a,int ac,unsigned char b,int bc){return itemCount(app,a)>=ac+(a==b?bc:0)&&(b==0||a==b||itemCount(app,b)>=bc);}
static bool craftRecipe(App& app,unsigned char a,int ac,unsigned char b,int bc,unsigned char out,int outc){if(!canCraft(app,a,ac,b,bc))return false;takeItem(app,a,ac);if(b&&bc>0)takeItem(app,b,bc);return addItem(app,out,outc);}
static const char* blockName(unsigned char type){static const char* names[BlockTypes]{"EMPTY","GRASS","DIRT","STONE","SNOW","WOOD","WATER","LEAVES","SAND","TALL GRASS","COAL ORE","IRON ORE","GOLD ORE","PLANKS"};return names[std::clamp(static_cast<int>(type),0,BlockTypes-1)];}
static void sortInventory(App& app){std::sort(app.inventory,app.inventory+InventorySlots,[](const Slot& a,const Slot& b){if(a.count==0&&b.count==0)return false;if(a.count==0)return false;if(b.count==0)return true;if(a.type!=b.type)return a.type<b.type;return a.count>b.count;});app.selected=std::clamp(app.selected,0,HotbarSlots-1);}
static bool playerInWater(const std::vector<Chunk>& chunks,Vec3 p){int x=static_cast<int>(std::floor(p.x));int z=static_cast<int>(std::floor(p.z));int y0=static_cast<int>(std::floor(p.y+0.25f));int y1=static_cast<int>(std::floor(p.y+1.55f));return isWater(getBlock(chunks,x,y0,z))||isWater(getBlock(chunks,x,y1,z));}
static bool cameraInWater(const std::vector<Chunk>& chunks,Vec3 p){int x=static_cast<int>(std::floor(p.x));int y=static_cast<int>(std::floor(p.y));int z=static_cast<int>(std::floor(p.z));return isWater(getBlock(chunks,x,y,z));}
static bool rainBlocked(const std::vector<Chunk>& chunks,Vec3 p){int x=static_cast<int>(std::floor(p.x));int z=static_cast<int>(std::floor(p.z));int y=std::clamp(static_cast<int>(std::floor(p.y+0.2f)),0,WorldHeight-1);for(int yy=y;yy<WorldHeight;yy++)if(isSolid(getBlock(chunks,x,yy,z)))return true;return false;}
static bool rainHit(const std::vector<Chunk>& chunks,Vec3 p){int x=static_cast<int>(std::floor(p.x));int y=static_cast<int>(std::floor(p.y));int z=static_cast<int>(std::floor(p.z));unsigned char t=getBlock(chunks,x,y,z);return y<0||(t!=0&&!isPlant(t));}
static bool traceSolid(const std::vector<Chunk>& chunks,Vec3 a,Vec3 b,Vec3& hit,unsigned char& type){Vec3 d=b-a;float dist=len(d);int steps=std::max(1,static_cast<int>(dist/0.16f));for(int i=1;i<=steps;i++){float t=static_cast<float>(i)/static_cast<float>(steps);Vec3 p=a+d*t;int x=static_cast<int>(std::floor(p.x));int y=static_cast<int>(std::floor(p.y));int z=static_cast<int>(std::floor(p.z));unsigned char block=getBlock(chunks,x,y,z);if(block!=0&&!isPlant(block)){hit=p;type=block;return true;}}return false;}
static bool visibleFromCamera(const std::vector<Chunk>& chunks,Vec3 a,Vec3 b){Vec3 hit;unsigned char type=0;return !traceSolid(chunks,a,b,hit,type);}
static float cameraHeight(const App& app){return app.player.crouching?1.18f:1.62f;}
static float playerBodyHeight(const App& app){return app.player.crouching?1.34f:1.78f;}
static Vec3 playerEye(const App& app){return app.player.p+Vec3{0.0f,cameraHeight(app),0.0f};}
static void updateCamera(App& app,float bob=0.0f){Vec3 eye=playerEye(app)+Vec3{0.0f,bob,0.0f};if(app.thirdPerson&&app.chunks){Vec3 back=cameraForward(app.camera)*-1.0f;Vec3 desired=eye+Vec3{0.0f,0.38f,0.0f}+back*4.4f;Vec3 hit;unsigned char type=0;if(traceSolid(*app.chunks,eye,desired,hit,type)){float dist=std::max(0.62f,len(hit-eye)-0.34f);desired=eye+norm(desired-eye)*dist;}app.camera.p=desired;}else app.camera.p=eye;}
static void resetRainParticle(App& app,RainParticle& r,int i,float t){int k=static_cast<int>(t*29.0f)+i*17;float rx=rand2(k*13+i,k*31-i);float rz=rand2(k*17-i,k*23+i);float ry=fract(std::sin(static_cast<float>(k)*18.13f)*9173.41f);r.p={app.camera.p.x+rx*31.0f,std::min(63.0f,app.camera.p.y+20.0f+ry*15.0f),app.camera.p.z+rz*31.0f};r.speed=17.0f+fract(std::sin(static_cast<float>(k)*7.31f)*731.17f)*10.0f;r.drift=rand2(k*5+i,k*7-i);}
static void addPuddle(App& app,Vec3 p,unsigned char type){if(type==6||type==7||type==5)return;for(Puddle& q:app.puddles){float dx=q.p.x-p.x;float dz=q.p.z-p.z;if(dx*dx+dz*dz<1.8f){q.radius=std::min(1.45f,q.radius+0.020f);q.life=9.0f;q.strength=std::min(1.0f,q.strength+0.075f);return;}}if(app.puddles.size()<80)app.puddles.push_back({p,0.16f,8.0f,0.22f});}
static void updateRain(App& app,float dt){if(!app.chunks)return;app.playerSheltered=app.rain&&rainBlocked(*app.chunks,playerEye(app));for(size_t i=0;i<app.puddles.size();){app.puddles[i].life-=dt;app.puddles[i].strength=std::max(0.0f,app.puddles[i].strength-dt*0.035f);if(app.puddles[i].life<=0.0f||app.puddles[i].strength<=0.0f)app.puddles.erase(app.puddles.begin()+static_cast<std::ptrdiff_t>(i));else i++;}if(!app.rain){app.rainParticles.clear();return;}float t=static_cast<float>(glfwGetTime());if(app.rainParticles.size()<220){size_t old=app.rainParticles.size();app.rainParticles.resize(220);for(size_t i=old;i<app.rainParticles.size();i++)resetRainParticle(app,app.rainParticles[i],static_cast<int>(i),t);}for(size_t i=0;i<app.rainParticles.size();i++){RainParticle& r=app.rainParticles[i];Vec3 old=r.p;r.p.x+=(-2.8f+r.drift*0.9f)*dt;r.p.y-=r.speed*dt;r.p.z+=(1.2f+r.drift*0.6f)*dt;Vec3 hit;unsigned char type=0;float dx=r.p.x-app.camera.p.x;float dz=r.p.z-app.camera.p.z;if(traceSolid(*app.chunks,old,r.p,hit,type)){int y=static_cast<int>(std::floor(hit.y));addPuddle(app,{hit.x,static_cast<float>(y)+1.01f,hit.z},type);resetRainParticle(app,r,static_cast<int>(i),t);}else if(r.p.y<1.0f||dx*dx+dz*dz>1225.0f||rainBlocked(*app.chunks,r.p)||rainHit(*app.chunks,r.p))resetRainParticle(app,r,static_cast<int>(i),t);}}
static void transferSlot(App& app,int from,int to){if(from<0||to<0||from>=InventorySlots||to>=InventorySlots||from==to)return;Slot& a=app.inventory[from];Slot& b=app.inventory[to];if(a.count>0&&b.count>0&&a.type==b.type&&b.count<64){int take=std::min(a.count,64-b.count);b.count+=take;a.count-=take;if(a.count<=0)a={0,0,1.0f};}else std::swap(a,b);}
static int inventorySlotAt(double mx,double my,int w,int h){float fw=static_cast<float>(std::max(w,1));float fh=static_cast<float>(std::max(h,1));float slot=std::clamp(fh*0.080f,52.0f,72.0f);float gap=slot*0.12f;float total=slot*HotbarSlots+gap*(HotbarSlots-1);float hotX=(fw-total)*0.5f;float hotY=fh-slot-28.0f;float invSlot=slot*0.95f;float invGap=invSlot*0.16f;float invTotal=invSlot*HotbarSlots+invGap*(HotbarSlots-1);float invX=(fw-invTotal)*0.5f;float invY=fh*0.16f+48.0f;float x=static_cast<float>(mx);float y=static_cast<float>(my);for(int row=0;row<3;row++){for(int col=0;col<HotbarSlots;col++){float x0=invX+static_cast<float>(col)*(invSlot+invGap);float y0=invY+static_cast<float>(row)*(invSlot+invGap);if(x>=x0&&x<=x0+invSlot&&y>=y0&&y<=y0+invSlot)return row*HotbarSlots+col;}}for(int col=0;col<HotbarSlots;col++){float x0=hotX+static_cast<float>(col)*(slot+gap);if(x>=x0&&x<=x0+slot&&y>=hotY&&y<=hotY+slot)return col;}return -1;}
static void updateInventoryMouse(GLFWwindow* window,App& app,float dt){int w=0;int h=0;glfwGetFramebufferSize(window,&w,&h);bool down=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;bool right=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS;if(right&&!app.lastInvRight){int slot=inventorySlotAt(app.mouseX,app.mouseY,w,h);if(slot>=0&&slot<InventorySlots&&app.inventory[slot].count>1){for(int i=0;i<InventorySlots;i++){if(i!=slot&&app.inventory[i].count==0){int take=app.inventory[slot].count/2;app.inventory[slot].count-=take;app.inventory[i]={app.inventory[slot].type,take,app.inventory[slot].wear};break;}}}}app.lastInvRight=right;if(down&&!app.lastMouseLeft){app.mouseDownSlot=inventorySlotAt(app.mouseX,app.mouseY,w,h);app.dragSlot=-1;app.mouseHold=0.0f;}if(down&&app.mouseDownSlot>=0){app.mouseHold+=dt;if(app.mouseHold>0.14f)app.dragSlot=app.mouseDownSlot;}if(!down&&app.lastMouseLeft){int slot=inventorySlotAt(app.mouseX,app.mouseY,w,h);if(app.dragSlot>=0&&slot>=0&&slot<InventorySlots&&slot!=app.dragSlot)transferSlot(app,app.dragSlot,slot);else if(slot>=0&&slot<HotbarSlots)app.selected=slot;app.dragSlot=-1;app.mouseDownSlot=-1;app.mouseHold=0.0f;}app.lastMouseLeft=down;}
static void setCursorMode(GLFWwindow* window,const App& app){glfwSetInputMode(window,GLFW_CURSOR,(app.inventoryOpen||app.debugOpen)?GLFW_CURSOR_NORMAL:GLFW_CURSOR_DISABLED);}
static void scrollCallback(GLFWwindow* window,double xoffset,double yoffset){if(ImGui::GetCurrentContext())ImGui_ImplGlfw_ScrollCallback(window,xoffset,yoffset);auto* app=static_cast<App*>(glfwGetWindowUserPointer(window));if(!app||app->inventoryOpen||app->debugOpen)return;if(yoffset>0.0)app->selected=(app->selected+HotbarSlots-1)%HotbarSlots;if(yoffset<0.0)app->selected=(app->selected+1)%HotbarSlots;}
static void cursorCallback(GLFWwindow* window,double xpos,double ypos){if(ImGui::GetCurrentContext())ImGui_ImplGlfw_CursorPosCallback(window,xpos,ypos);auto* app=static_cast<App*>(glfwGetWindowUserPointer(window));if(!app)return;app->mouseX=xpos;app->mouseY=ypos;if(app->inventoryOpen||app->debugOpen){app->camera.firstMouse=true;return;}Camera& c=app->camera;if(c.firstMouse){c.lastX=xpos;c.lastY=ypos;c.firstMouse=false;}float dx=static_cast<float>(xpos-c.lastX);float dy=static_cast<float>(c.lastY-ypos);c.lastX=xpos;c.lastY=ypos;float sens=0.09f;c.yaw+=dx*sens;c.pitch=std::clamp(c.pitch+dy*sens,-84.0f,84.0f);}
static void mouseButtonCallback(GLFWwindow* window,int button,int action,int mods){if(ImGui::GetCurrentContext())ImGui_ImplGlfw_MouseButtonCallback(window,button,action,mods);}
static void keyCallback(GLFWwindow* window,int key,int scancode,int action,int mods){if(ImGui::GetCurrentContext())ImGui_ImplGlfw_KeyCallback(window,key,scancode,action,mods);}
static void charCallback(GLFWwindow* window,unsigned int c){if(ImGui::GetCurrentContext())ImGui_ImplGlfw_CharCallback(window,c);}
static bool pressed(GLFWwindow* window,int key){return glfwGetKey(window,key)==GLFW_PRESS;}
static void moveAxis(App& app,float dx,float dy,float dz,bool step){if(!app.chunks)return;Vec3 old=app.player.p;Vec3 next{old.x+dx,old.y+dy,old.z+dz};float h=playerBodyHeight(app);if(!playerHitsHeight(*app.chunks,next,h)){app.player.p=next;return;}if(step&&(dx!=0.0f||dz!=0.0f)){Vec3 up{old.x+dx,old.y+1.02f,old.z+dz};if(!playerHitsHeight(*app.chunks,up,h)){app.player.p=up;return;}}if(dx!=0.0f)app.player.v.x=0.0f;if(dy!=0.0f){if(dy<0.0f)app.player.grounded=true;app.player.v.y=0.0f;}if(dz!=0.0f)app.player.v.z=0.0f;}
static void respawnPlayer(App& app){app.player.p=findSpawn();ensureChunks(*app.chunks,app.player.p,app.renderDistance);keepPlayerClear(app);app.player.v={0.0f,0.0f,0.0f};app.player.health=1.0f;app.player.stamina=1.0f;app.player.oxygen=1.0f;app.player.crouching=false;app.breakTimer=0.0f;app.breakProgress=0.0f;for(int i=0;i<7;i++){app.tailYaw[i]=0.0f;app.tailYawVel[i]=0.0f;app.tailPitch[i]=-0.48f;app.tailPitchVel[i]=0.0f;}}
static void updateAvatarTail(App& app,float dt,float speed){float t=static_cast<float>(glfwGetTime());float move=std::clamp(speed/7.2f,0.0f,1.0f);float swim=app.player.swimming?1.0f:0.0f;float crouch=app.player.crouching?1.0f:0.0f;float air=(!app.player.grounded&&!app.fly&&!app.player.swimming)?1.0f:0.0f;for(int i=0;i<7;i++){float k=static_cast<float>(i);float targetYaw=app.avatarAnim?(std::sin(t*(2.6f+move*4.2f+swim*1.5f)-k*(0.50f+swim*0.18f))*(0.14f+move*0.34f+swim*0.22f)+std::sin(t*1.15f+k*0.37f)*0.05f):0.0f;float targetPitch=app.avatarAnim?(-0.50f+std::sin(t*(1.8f+move*2.2f+swim*2.4f)-k*0.42f)*0.08f+move*0.10f+swim*0.30f-crouch*0.18f-air*0.08f):-0.48f;float spring=13.0f+k*1.4f+swim*2.0f;app.tailYawVel[i]+=(targetYaw-app.tailYaw[i])*spring*dt;app.tailPitchVel[i]+=(targetPitch-app.tailPitch[i])*spring*dt;float damp=std::max(0.0f,1.0f-dt*(6.0f+k*0.18f+swim*1.2f));app.tailYawVel[i]*=damp;app.tailPitchVel[i]*=damp;app.tailYaw[i]+=app.tailYawVel[i]*dt;app.tailPitch[i]+=app.tailPitchVel[i]*dt;}}
static void updateInput(GLFWwindow* window,App& app,float dt){
if(!app.chunks)return;
if(pressed(window,GLFW_KEY_ESCAPE))glfwSetWindowShouldClose(window,GLFW_TRUE);
bool tabNow=pressed(window,GLFW_KEY_TAB);
if(tabNow&&!app.lastTab){app.inventoryOpen=!app.inventoryOpen;app.dragSlot=-1;app.mouseDownSlot=-1;app.lastMouseLeft=false;app.lastInvRight=false;setCursorMode(window,app);app.camera.firstMouse=true;}
app.lastTab=tabNow;
bool debugNow=pressed(window,GLFW_KEY_F3);
if(debugNow&&!app.lastDebug){app.debugOpen=!app.debugOpen;setCursorMode(window,app);app.camera.firstMouse=true;}
app.lastDebug=debugNow;
bool viewNow=pressed(window,GLFW_KEY_F4);
if(viewNow&&!app.lastView){app.thirdPerson=!app.thirdPerson;app.avatarAnim=true;app.camera.firstMouse=true;}
app.lastView=viewNow;
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
bool rainNow=pressed(window,GLFW_KEY_P);
if(rainNow&&!app.lastRain)app.rain=!app.rain;
app.lastRain=rainNow;
bool resetNow=pressed(window,GLFW_KEY_R);
if(resetNow&&!app.lastReset){clearChunks(*app.chunks);resetInventory(app);respawnPlayer(app);}
app.lastReset=resetNow;
if(app.inventoryOpen||app.debugOpen){if(app.inventoryOpen)updateInventoryMouse(window,app,dt);app.player.v.x=0.0f;app.player.v.z=0.0f;app.breakTimer=0.0f;app.breakProgress=0.0f;updateAvatarTail(app,dt,0.0f);updateCamera(app);app.lastPlace=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS;app.lastMouseLeft=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;return;}
int keys[HotbarSlots]{GLFW_KEY_1,GLFW_KEY_2,GLFW_KEY_3,GLFW_KEY_4,GLFW_KEY_5,GLFW_KEY_6,GLFW_KEY_7,GLFW_KEY_8,GLFW_KEY_9};
for(int i=0;i<HotbarSlots;i++){bool down=pressed(window,keys[i]);if(down&&!app.lastSlot[i])app.selected=i;app.lastSlot[i]=down;}
bool wantCrouch=pressed(window,GLFW_KEY_C)&&app.player.grounded&&!playerInWater(*app.chunks,app.player.p)&&!app.fly&&!app.noclip;
if(!wantCrouch&&app.player.crouching&&playerHitsHeight(*app.chunks,app.player.p,1.78f))wantCrouch=true;
app.player.crouching=wantCrouch;
Vec3 eye=playerEye(app);
Vec3 dir=cameraForward(app.camera);
bool breakNow=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;
if(breakNow){RayHit hit=raycast(*app.chunks,eye,dir);if(hit.hit&&hit.y>0){if(hit.x!=app.breakX||hit.y!=app.breakY||hit.z!=app.breakZ){app.breakX=hit.x;app.breakY=hit.y;app.breakZ=hit.z;app.breakTimer=0.0f;}float hardness=app.creative?0.04f:std::max(0.1f,blockHardness(hit.type));app.breakTimer+=dt;app.breakProgress=std::clamp(app.breakTimer/hardness,0.0f,1.0f);if(app.breakProgress>=1.0f){setBlock(*app.chunks,hit.x,hit.y,hit.z,0);addItem(app,hit.type,1);app.breakTimer=0.0f;app.breakProgress=0.0f;}}else{app.breakTimer=0.0f;app.breakProgress=0.0f;}}else{app.breakTimer=0.0f;app.breakProgress=0.0f;}
bool placeNow=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS;
if(placeNow&&!app.lastPlace){RayHit hit=raycast(*app.chunks,eye,dir);Slot& slot=app.inventory[app.selected];if(hit.hit&&slot.count>0&&slot.type>0&&hit.py>=0&&hit.py<WorldHeight&&!isSolid(getBlock(*app.chunks,hit.px,hit.py,hit.pz))&&!blockTouchesPlayer(app.player.p,hit.px,hit.py,hit.pz)){setBlock(*app.chunks,hit.px,hit.py,hit.pz,slot.type);if(!app.creative){slot.count--;slot.wear=std::max(0.0f,slot.wear-0.03f);if(slot.count<=0)slot={0,0,1.0f};}}}
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
if(app.creative)app.player.stamina=1.0f;
else if(sprint)app.player.stamina=std::max(0.0f,app.player.stamina-staminaUse*dt);
else app.player.stamina=std::min(1.0f,app.player.stamina+staminaGain*dt);
float speed=app.player.swimming?(sprint?4.5f:3.2f):(sprint?7.2f:4.8f);
if(app.speedMode)speed*=2.2f;
if(app.player.crouching)speed*=0.45f;
updateAvatarTail(app,dt,moving?speed:0.0f);
app.player.v.x=wish.x*speed;
app.player.v.z=wish.z*speed;
float vertical=0.0f;
if(pressed(window,GLFW_KEY_SPACE))vertical+=1.0f;
if(pressed(window,GLFW_KEY_LEFT_CONTROL)||pressed(window,GLFW_KEY_RIGHT_CONTROL))vertical-=1.0f;
if(app.noclip){app.player.p+=Vec3{wish.x*speed,vertical*speed,wish.z*speed}*dt;app.player.v={0.0f,0.0f,0.0f};app.player.grounded=false;app.player.swimming=false;updateCamera(app);return;}
if(app.fly){app.player.v.y=vertical*speed;app.player.grounded=false;}
else if(app.player.swimming){if(vertical>0.0f)app.player.v.y+=10.0f*dt;if(vertical<0.0f)app.player.v.y-=8.0f*dt;app.player.v.y=std::clamp(app.player.v.y-3.0f*dt,-3.8f,3.2f);app.player.grounded=false;}
else{if(pressed(window,GLFW_KEY_SPACE)&&app.player.grounded){app.player.v.y=8.2f;app.player.grounded=false;}app.player.v.y-=22.0f*dt;}
bool wasGrounded=app.player.grounded;
float fallVelocity=app.player.v.y;
bool canStep=app.player.grounded&&!app.player.swimming&&!app.fly;
app.player.grounded=false;
float dx=app.player.v.x*dt;
float dz=app.player.v.z*dt;
if(app.player.crouching&&wasGrounded&&!hasFooting(*app.chunks,app.player.p+Vec3{dx,0.0f,0.0f}))dx=0.0f;
if(app.player.crouching&&wasGrounded&&!hasFooting(*app.chunks,app.player.p+Vec3{0.0f,0.0f,dz}))dz=0.0f;
moveAxis(app,dx,0.0f,0.0f,canStep);
moveAxis(app,0.0f,0.0f,dz,canStep);
moveAxis(app,0.0f,app.player.v.y*dt,0.0f,false);
if(!wasGrounded&&app.player.grounded&&fallVelocity<-13.0f&&!app.creative&&!app.fly){app.player.health=std::max(0.0f,app.player.health-std::min(0.80f,(-fallVelocity-13.0f)*0.055f));if(app.player.health<=0.0f)respawnPlayer(app);}
if(app.player.p.y<-16.0f)respawnPlayer(app);
if(app.player.grounded&&moving&&!app.player.swimming)app.bobPhase+=dt*speed*(app.player.crouching?2.8f:4.2f);else app.bobPhase*=std::max(0.0f,1.0f-dt*6.0f);
float bob=(app.player.grounded&&moving&&!app.player.swimming)?std::sin(app.bobPhase)*0.028f:0.0f;
updateCamera(app,bob);
bool headUnder=cameraInWater(*app.chunks,app.camera.p);
if(app.creative||!headUnder)app.player.oxygen=std::min(1.0f,app.player.oxygen+0.55f*dt);
else{app.player.oxygen=std::max(0.0f,app.player.oxygen-0.16f*dt);if(app.player.oxygen<=0.0f)app.player.health=std::max(0.0f,app.player.health-0.10f*dt);}
if(!headUnder&&!app.player.swimming&&app.player.grounded&&app.player.stamina>0.84f&&app.player.health<1.0f)app.player.health=std::min(1.0f,app.player.health+0.025f*dt);
if(app.player.health<=0.0f){respawnPlayer(app);updateCamera(app);}
}
static ImU32 imguiColor(Vec3 c,float a=1.0f){return ImGui::ColorConvertFloat4ToU32({std::clamp(c.x,0.0f,1.0f),std::clamp(c.y,0.0f,1.0f),std::clamp(c.z,0.0f,1.0f),a});}
static void drawImGuiBar(ImDrawList* d,ImVec2 p,ImVec2 q,float value,ImU32 bg,ImU32 fg){float v=std::clamp(value,0.0f,1.0f);d->AddRectFilled(p,q,bg,6.0f);d->AddRectFilled(p,{p.x+(q.x-p.x)*v,q.y},fg,6.0f);d->AddRect(p,q,imguiColor({0.02f,0.025f,0.03f}),6.0f,0,2.0f);}
static void drawImGuiIcon(ImDrawList* d,ImVec2 p,ImVec2 q,unsigned char type){Vec3 c=itemColor(type);float w=q.x-p.x;float h=q.y-p.y;d->AddRectFilled(p,q,imguiColor(c*0.82f),5.0f);d->AddRectFilled({p.x+w*0.07f,p.y+h*0.08f},{q.x-w*0.07f,q.y-h*0.08f},imguiColor(c*1.10f),4.0f);d->AddRectFilled({p.x+w*0.07f,p.y+h*0.08f},{q.x-w*0.07f,p.y+h*0.22f},imguiColor(c*1.26f),4.0f);if(type==1){d->AddRectFilled({p.x+w*0.12f,p.y+h*0.10f},{q.x-w*0.12f,p.y+h*0.34f},imguiColor({0.18f,0.74f,0.20f}),3.0f);d->AddRectFilled({p.x+w*0.16f,p.y+h*0.60f},{q.x-w*0.16f,p.y+h*0.78f},imguiColor({0.38f,0.24f,0.12f}),2.0f);}else if(type==2){for(int i=0;i<4;i++)d->AddRectFilled({p.x+w*(0.14f+0.19f*i),p.y+h*0.22f},{p.x+w*(0.22f+0.19f*i),q.y-h*0.18f},imguiColor({0.30f,0.18f,0.10f}),2.0f);}else if(type==3){d->AddRectFilled({p.x+w*0.17f,p.y+h*0.18f},{p.x+w*0.46f,p.y+h*0.48f},imguiColor({0.25f,0.26f,0.26f}),3.0f);d->AddRectFilled({p.x+w*0.55f,p.y+h*0.52f},{p.x+w*0.84f,p.y+h*0.80f},imguiColor({0.60f,0.60f,0.58f}),3.0f);}else if(type==4){d->AddRectFilled({p.x+w*0.10f,p.y+h*0.16f},{q.x-w*0.10f,p.y+h*0.50f},imguiColor({1.0f,1.0f,0.96f}),4.0f);d->AddRectFilled({p.x+w*0.18f,p.y+h*0.58f},{q.x-w*0.20f,p.y+h*0.72f},imguiColor({0.80f,0.86f,0.90f}),2.0f);}else if(type==5){for(int i=0;i<5;i++)d->AddRectFilled({p.x+w*0.13f,p.y+h*(0.18f+0.14f*i)},{q.x-w*0.13f,p.y+h*(0.23f+0.14f*i)},imguiColor({0.25f,0.14f,0.06f}),2.0f);d->AddRectFilled({p.x+w*0.46f,p.y+h*0.13f},{p.x+w*0.56f,q.y-h*0.13f},imguiColor({0.58f,0.36f,0.16f}),2.0f);}else if(type==6){d->AddRectFilled({p.x+w*0.08f,p.y+h*0.14f},{q.x-w*0.08f,q.y-h*0.12f},imguiColor({0.12f,0.48f,0.86f}),5.0f);for(int i=0;i<4;i++)d->AddLine({p.x+w*0.15f,p.y+h*(0.30f+0.13f*i)},{q.x-w*0.14f,p.y+h*(0.25f+0.13f*i)},imguiColor({0.58f,0.88f,1.0f}),2.0f);}else if(type==7){d->AddRectFilled({p.x+w*0.13f,p.y+h*0.18f},{p.x+w*0.52f,p.y+h*0.55f},imguiColor({0.14f,0.62f,0.17f}),5.0f);d->AddRectFilled({p.x+w*0.42f,p.y+h*0.40f},{p.x+w*0.86f,p.y+h*0.84f},imguiColor({0.10f,0.44f,0.12f}),5.0f);}else if(type==8){for(int i=0;i<4;i++)d->AddRectFilled({p.x+w*(0.13f+0.18f*i),p.y+h*0.28f},{p.x+w*(0.22f+0.18f*i),p.y+h*0.43f},imguiColor({0.94f,0.84f,0.50f}),2.0f);d->AddRectFilled({p.x+w*0.12f,p.y+h*0.60f},{q.x-w*0.12f,p.y+h*0.72f},imguiColor({0.62f,0.54f,0.32f}),2.0f);}else if(type==9){for(int i=0;i<5;i++)d->AddLine({p.x+w*(0.20f+0.13f*i),q.y-h*0.16f},{p.x+w*(0.32f+0.07f*i),p.y+h*(0.24f+0.05f*i)},imguiColor({0.20f,0.86f,0.18f}),2.0f);}else if(type==10||type==11||type==12){ImU32 ore=type==10?imguiColor({0.04f,0.04f,0.04f}):(type==11?imguiColor({0.82f,0.62f,0.44f}):imguiColor({1.0f,0.76f,0.18f}));for(int i=0;i<5;i++)d->AddCircleFilled({p.x+w*(0.24f+0.15f*(i%3)),p.y+h*(0.28f+0.19f*(i/2))},w*0.055f,ore,10);}else if(type==13){for(int i=0;i<4;i++)d->AddRectFilled({p.x+w*0.12f,p.y+h*(0.22f+0.14f*i)},{q.x-w*0.12f,p.y+h*(0.28f+0.14f*i)},imguiColor({0.78f,0.50f,0.24f}),2.0f);}}
static void drawImGuiSlot(ImDrawList* d,const Slot& s,ImVec2 p,float size,bool selected){ImVec2 q{p.x+size,p.y+size};if(selected)d->AddRectFilled({p.x-5.0f,p.y-5.0f},{q.x+5.0f,q.y+5.0f},imguiColor({1.0f,0.76f,0.22f},0.18f),11.0f);d->AddRectFilled({p.x+4.0f,p.y+6.0f},{q.x+5.0f,q.y+7.0f},imguiColor({0.01f,0.012f,0.016f},0.60f),8.0f);d->AddRectFilled(p,q,selected?imguiColor({0.18f,0.20f,0.23f},0.96f):imguiColor({0.050f,0.058f,0.072f},0.92f),8.0f);d->AddRect({p.x+3.0f,p.y+3.0f},{q.x-3.0f,q.y-3.0f},imguiColor({0.95f,0.98f,1.0f},0.06f),6.0f,0,1.0f);d->AddRect(p,q,selected?imguiColor({1.0f,0.82f,0.26f}):imguiColor({0.21f,0.26f,0.32f}),8.0f,0,selected?3.0f:1.4f);ImVec2 ip{p.x+size*0.13f,p.y+size*0.13f};ImVec2 iq{q.x-size*0.13f,q.y-size*0.13f};if(s.count>0)drawImGuiIcon(d,ip,iq,s.type);if(s.count>0){drawImGuiBar(d,{p.x+size*0.14f,q.y-size*0.11f},{q.x-size*0.14f,q.y-size*0.06f},s.wear,imguiColor({0.05f,0.06f,0.07f}),s.wear>0.35f?imguiColor({0.28f,0.92f,0.42f}):imguiColor({0.92f,0.34f,0.22f}));if(s.count>1){std::string count=std::to_string(s.count);ImVec2 ts=ImGui::CalcTextSize(count.c_str());ImVec2 bp{q.x-ts.x-14.0f,q.y-ts.y-12.0f};d->AddRectFilled({bp.x-4.0f,bp.y-2.0f},{q.x-5.0f,q.y-5.0f},imguiColor({0.02f,0.024f,0.030f},0.82f),5.0f);d->AddText({bp.x+1.0f,bp.y+1.0f},imguiColor({0.0f,0.0f,0.0f}),count.c_str());d->AddText(bp,imguiColor({1.0f,0.96f,0.76f}),count.c_str());}}}
static void drawCraftButton(App& app,const char* label,unsigned char a,int ac,unsigned char b,int bc,unsigned char out,int outc){bool can=canCraft(app,a,ac,b,bc);if(!can)ImGui::BeginDisabled();if(ImGui::Button(label,{220.0f,30.0f}))craftRecipe(app,a,ac,b,bc,out,outc);if(!can)ImGui::EndDisabled();ImGui::Text("%s x%d",blockName(out),outc);}
static void drawCraftingPanel(App& app,int w,int h){if(!app.inventoryOpen)return;ImGui::SetNextWindowPos({std::max(18.0f,static_cast<float>(w)-292.0f),static_cast<float>(h)*0.16f},ImGuiCond_Always);ImGui::SetNextWindowSize({268.0f,236.0f},ImGuiCond_Always);if(ImGui::Begin("Crafting",nullptr,ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoResize)){drawCraftButton(app,"Wood -> Planks",5,1,0,0,13,4);drawCraftButton(app,"Planks -> Wood",13,2,0,0,5,1);drawCraftButton(app,"Dirt + Grass",2,2,9,2,1,1);drawCraftButton(app,"Leaves -> Grass",7,4,0,0,9,3);}ImGui::End();}
static void drawDebugPanel(App& app){if(!app.debugOpen)return;ImGui::SetNextWindowPos({18.0f,18.0f},ImGuiCond_FirstUseEver);ImGui::SetNextWindowSize({318.0f,450.0f},ImGuiCond_FirstUseEver);if(ImGui::Begin("Debug")){ImGui::Checkbox("Speed",&app.speedMode);ImGui::SameLine();ImGui::Checkbox("Creative",&app.creative);ImGui::Checkbox("Fly",&app.fly);ImGui::SameLine();ImGui::Checkbox("Noclip",&app.noclip);ImGui::Checkbox("Third person",&app.thirdPerson);ImGui::SameLine();ImGui::Checkbox("Avatar anim",&app.avatarAnim);ImGui::Separator();ImGui::Checkbox("Wireframe",&app.wireframe);ImGui::Checkbox("Clouds",&app.clouds);ImGui::Checkbox("Water effect",&app.waterEffect);ImGui::Checkbox("Rain",&app.rain);ImGui::Checkbox("Day cycle",&app.dayCycle);ImGui::SliderFloat("Day time",&app.dayTime,0.0f,1.0f);ImGui::SliderInt("Render distance",&app.renderDistance,2,6);ImGui::SliderFloat("Fog distance",&app.fogDistance,70.0f,320.0f);ImGui::Separator();int bx=static_cast<int>(std::floor(app.player.p.x));int bz=static_cast<int>(std::floor(app.player.p.z));ImGui::Text("Biome %s",biomeName(biomeAt(bx,bz)));ImGui::Text("XYZ %.1f %.1f %.1f",app.player.p.x,app.player.p.y,app.player.p.z);ImGui::Text("Health %.0f%%",app.player.health*100.0f);ImGui::ProgressBar(app.player.oxygen,{220.0f,0.0f},"Oxygen");ImGui::Text("State %s%s%s",app.player.swimming?"Swimming ":"",app.player.crouching?"Crouch ":"",app.rain?"Rain":"");ImGui::Text("Chunks %d",app.chunks?static_cast<int>(app.chunks->size()):0);ImGui::Text("Selected %s",blockName(app.inventory[app.selected].type));}ImGui::End();}
static bool worldToScreen(Vec3 p,Mat4 mvp,int w,int h,ImVec2& out){float x=mvp.m[0]*p.x+mvp.m[4]*p.y+mvp.m[8]*p.z+mvp.m[12];float y=mvp.m[1]*p.x+mvp.m[5]*p.y+mvp.m[9]*p.z+mvp.m[13];float z=mvp.m[2]*p.x+mvp.m[6]*p.y+mvp.m[10]*p.z+mvp.m[14];float q=mvp.m[3]*p.x+mvp.m[7]*p.y+mvp.m[11]*p.z+mvp.m[15];if(q<=0.02f)return false;float nx=x/q;float ny=y/q;float nz=z/q;if(nx<-1.25f||nx>1.25f||ny<-1.25f||ny>1.25f||nz<-1.0f||nz>1.0f)return false;out={static_cast<float>(w)*(nx*0.5f+0.5f),static_cast<float>(h)*(0.5f-ny*0.5f)};return true;}
static void drawBlockHighlight(ImDrawList* d,const RayHit& hit,Mat4 mvp,int w,int h){float xs[2]{static_cast<float>(hit.x),static_cast<float>(hit.x+1)};float ys[2]{static_cast<float>(hit.y),static_cast<float>(hit.y+1)};float zs[2]{static_cast<float>(hit.z),static_cast<float>(hit.z+1)};ImVec2 mn{static_cast<float>(w),static_cast<float>(h)};ImVec2 mx{0.0f,0.0f};int count=0;for(float x:xs)for(float y:ys)for(float z:zs){ImVec2 p;if(worldToScreen({x,y,z},mvp,w,h,p)){mn.x=std::min(mn.x,p.x);mn.y=std::min(mn.y,p.y);mx.x=std::max(mx.x,p.x);mx.y=std::max(mx.y,p.y);count++;}}if(count<2)return;float len=std::clamp(std::min(mx.x-mn.x,mx.y-mn.y)*0.22f,6.0f,22.0f);ImU32 c=imguiColor({1.0f,0.82f,0.30f},0.72f);d->AddLine(mn,{mn.x+len,mn.y},c,1.5f);d->AddLine(mn,{mn.x,mn.y+len},c,1.5f);d->AddLine({mx.x,mn.y},{mx.x-len,mn.y},c,1.5f);d->AddLine({mx.x,mn.y},{mx.x,mn.y+len},c,1.5f);d->AddLine({mn.x,mx.y},{mn.x+len,mx.y},c,1.5f);d->AddLine({mn.x,mx.y},{mn.x,mx.y-len},c,1.5f);d->AddLine(mx,{mx.x-len,mx.y},c,1.5f);d->AddLine(mx,{mx.x,mx.y-len},c,1.5f);}
static void drawImGuiHud(App& app,int w,int h){ImGui::SetNextWindowPos({0.0f,0.0f});ImGui::SetNextWindowSize({static_cast<float>(w),static_cast<float>(h)});ImGui::Begin("WorldOverlay",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoInputs|ImGuiWindowFlags_NoBackground);ImDrawList* d=ImGui::GetWindowDrawList();if(app.chunks&&cameraInWater(*app.chunks,app.camera.p)&&app.waterEffect){d->AddRectFilled({0.0f,0.0f},{static_cast<float>(w),static_cast<float>(h)},imguiColor({0.02f,0.22f,0.48f},0.34f));float t=static_cast<float>(ImGui::GetTime());for(int i=0;i<9;i++){float yy=static_cast<float>(h)*(0.12f+0.09f*i)+std::sin(t*1.7f+static_cast<float>(i))*8.0f;d->AddLine({0.0f,yy},{static_cast<float>(w),yy+std::sin(t*1.3f+static_cast<float>(i))*24.0f},imguiColor({0.50f,0.86f,1.0f},0.18f),2.0f);}for(int i=0;i<22;i++){float x=fract(std::sin(static_cast<float>(i)*12.989f+t*0.17f)*43758.54f)*static_cast<float>(w);float y=fract(std::sin(static_cast<float>(i)*78.233f+t*0.23f)*24634.63f)*static_cast<float>(h);d->AddCircle({x,y},2.0f+static_cast<float>(i%5),imguiColor({0.72f,0.94f,1.0f},0.30f),12,1.4f);}}float slot=std::clamp(static_cast<float>(h)*0.080f,52.0f,72.0f);float gap=slot*0.12f;float total=slot*HotbarSlots+gap*(HotbarSlots-1);float start=(static_cast<float>(w)-total)*0.5f;float y=static_cast<float>(h)-slot-28.0f;for(int i=0;i<HotbarSlots;i++)drawImGuiSlot(d,app.inventory[i],{start+static_cast<float>(i)*(slot+gap),y},slot,i==app.selected);ImVec2 c{static_cast<float>(w)*0.5f,static_cast<float>(h)*0.5f};if(!app.inventoryOpen&&!app.debugOpen){d->AddLine({c.x-8.0f,c.y},{c.x+8.0f,c.y},imguiColor({0.95f,0.95f,0.88f}),1.5f);d->AddLine({c.x,c.y-8.0f},{c.x,c.y+8.0f},imguiColor({0.95f,0.95f,0.88f}),1.5f);}d->AddText({24.0f,static_cast<float>(h)-142.0f},imguiColor({0.98f,0.92f,0.84f}),"HP");drawImGuiBar(d,{70.0f,static_cast<float>(h)-140.0f},{240.0f,static_cast<float>(h)-121.0f},app.player.health,imguiColor({0.14f,0.030f,0.035f},0.86f),imguiColor({0.92f,0.16f,0.14f}));d->AddText({24.0f,static_cast<float>(h)-112.0f},imguiColor({0.98f,0.92f,0.84f}),"STA");drawImGuiBar(d,{70.0f,static_cast<float>(h)-110.0f},{240.0f,static_cast<float>(h)-91.0f},app.player.stamina,imguiColor({0.08f,0.08f,0.06f},0.86f),imguiColor({0.92f,0.74f,0.24f}));int bx=static_cast<int>(std::floor(app.player.p.x));int bz=static_cast<int>(std::floor(app.player.p.z));std::string dist="DRAW "+std::to_string(app.renderDistance);d->AddText({24.0f,static_cast<float>(h)-78.0f},imguiColor({0.70f,0.84f,0.98f}),dist.c_str());std::string biomeText=std::string("BIOME ")+biomeName(biomeAt(bx,bz));d->AddText({24.0f,static_cast<float>(h)-58.0f},imguiColor({0.70f,0.92f,0.78f}),biomeText.c_str());std::string chunkText="CHUNKS "+std::to_string(app.chunks?static_cast<int>(app.chunks->size()):0);d->AddText({24.0f,static_cast<float>(h)-40.0f},imguiColor({0.78f,0.82f,0.90f}),chunkText.c_str());if(app.breakProgress>0.0f)drawImGuiBar(d,{static_cast<float>(w)*0.5f-130.0f,static_cast<float>(h)*0.5f+82.0f},{static_cast<float>(w)*0.5f+130.0f,static_cast<float>(h)*0.5f+100.0f},app.breakProgress,imguiColor({0.04f,0.045f,0.052f},0.88f),imguiColor({0.95f,0.72f,0.28f}));if(app.inventoryOpen){float invSlot=slot*0.95f;float invGap=invSlot*0.16f;float invTotal=invSlot*HotbarSlots+invGap*(HotbarSlots-1);ImVec2 panel{(static_cast<float>(w)-invTotal)*0.5f-30.0f,static_cast<float>(h)*0.16f};ImVec2 panelEnd{panel.x+invTotal+60.0f,panel.y+invSlot*3.0f+invGap*2.0f+70.0f};d->AddRectFilled({panel.x+8.0f,panel.y+10.0f},{panelEnd.x+8.0f,panelEnd.y+10.0f},imguiColor({0.01f,0.012f,0.016f},0.60f),14.0f);d->AddRectFilled(panel,panelEnd,imguiColor({0.050f,0.058f,0.070f},0.94f),14.0f);d->AddRect(panel,panelEnd,imguiColor({0.24f,0.30f,0.36f},0.95f),14.0f,0,1.6f);d->AddText({panel.x+24.0f,panel.y+18.0f},imguiColor({0.96f,0.88f,0.62f}),"Inventory");for(int row=0;row<3;row++){for(int col=0;col<HotbarSlots;col++){int idx=row*HotbarSlots+col;drawImGuiSlot(d,app.inventory[idx],{panel.x+30.0f+static_cast<float>(col)*(invSlot+invGap),panel.y+48.0f+static_cast<float>(row)*(invSlot+invGap)},invSlot,idx==app.selected);}}int hover=inventorySlotAt(app.mouseX,app.mouseY,w,h);if(hover>=0&&hover<InventorySlots&&hover!=app.dragSlot&&app.inventory[hover].count>0){const Slot& s=app.inventory[hover];ImVec2 m{static_cast<float>(app.mouseX),static_cast<float>(app.mouseY)};ImVec2 tp{std::min(m.x+18.0f,static_cast<float>(w)-230.0f),std::min(m.y+18.0f,static_cast<float>(h)-112.0f)};ImVec2 tq{tp.x+218.0f,tp.y+98.0f};d->AddRectFilled({tp.x+5.0f,tp.y+6.0f},{tq.x+5.0f,tq.y+6.0f},imguiColor({0.01f,0.012f,0.016f},0.65f),9.0f);d->AddRectFilled(tp,tq,imguiColor({0.050f,0.058f,0.070f},0.96f),9.0f);d->AddRect(tp,tq,imguiColor({0.30f,0.35f,0.40f}),9.0f);d->AddText({tp.x+14.0f,tp.y+12.0f},imguiColor({0.96f,0.88f,0.62f}),blockName(s.type));std::string cnt="Count: "+std::to_string(s.count);std::string hard="Hardness: "+std::to_string(static_cast<int>(blockHardness(s.type)*100.0f));d->AddText({tp.x+14.0f,tp.y+42.0f},imguiColor({0.82f,0.88f,0.94f}),cnt.c_str());d->AddText({tp.x+14.0f,tp.y+66.0f},imguiColor({0.82f,0.88f,0.94f}),hard.c_str());}if(app.dragSlot>=0&&app.dragSlot<InventorySlots&&app.inventory[app.dragSlot].count>0){ImVec2 m{static_cast<float>(app.mouseX),static_cast<float>(app.mouseY)};drawImGuiIcon(d,{m.x-invSlot*0.45f,m.y-invSlot*0.45f},{m.x+invSlot*0.45f,m.y+invSlot*0.45f},app.inventory[app.dragSlot].type);}}ImGui::End();drawCraftingPanel(app,w,h);drawDebugPanel(app);}
static void drawExtraHud(App& app,int w,int h){updateRain(app,ImGui::GetIO().DeltaTime);ImDrawList* d=ImGui::GetForegroundDrawList();float fw=static_cast<float>(w);float fh=static_cast<float>(h);float aspect=fw/std::max(fh,1.0f);Mat4 proj=perspective(65.0f*Pi/180.0f,aspect,0.1f,420.0f);Vec3 forward=cameraForward(app.camera);Mat4 view=lookAt(app.camera.p,app.camera.p+forward,{0.0f,1.0f,0.0f});Mat4 mvp=mul(proj,view);if(app.rain){float cover=app.playerSheltered?0.025f:0.12f;d->AddRectFilled({0.0f,0.0f},{fw,fh},imguiColor({0.03f,0.045f,0.070f},cover));for(const RainParticle& r:app.rainParticles){ImVec2 a,b;Vec3 tail{r.p.x-0.18f,r.p.y-0.85f,r.p.z+0.08f};if(visibleFromCamera(*app.chunks,app.camera.p,r.p)&&worldToScreen(r.p,mvp,w,h,a)&&worldToScreen(tail,mvp,w,h,b)){float dx=r.p.x-app.camera.p.x;float dz=r.p.z-app.camera.p.z;float dist=std::sqrt(dx*dx+dz*dz);float alpha=std::clamp(0.50f-dist*0.010f,0.10f,0.44f);d->AddLine(a,b,imguiColor({0.58f,0.72f,0.92f},alpha),1.15f);}}}if(app.chunks){for(const Puddle& q:app.puddles){ImVec2 a,b;if(q.strength<=0.02f||!visibleFromCamera(*app.chunks,app.camera.p,q.p)||!worldToScreen(q.p,mvp,w,h,a)||!worldToScreen({q.p.x+q.radius,q.p.y,q.p.z},mvp,w,h,b))continue;float rr=std::clamp(std::abs(b.x-a.x),3.0f,34.0f);float alpha=std::clamp(q.strength*0.34f,0.04f,0.32f);d->AddCircleFilled(a,rr,imguiColor({0.06f,0.26f,0.42f},alpha),28);d->AddCircle(a,rr,imguiColor({0.45f,0.78f,1.0f},alpha*0.9f),28,1.2f);d->AddCircle(a,rr*0.48f,imguiColor({0.72f,0.92f,1.0f},alpha*0.45f),20,1.0f);}}d->AddText({24.0f,24.0f},imguiColor({0.78f,0.92f,1.0f}),"OXY");drawImGuiBar(d,{70.0f,24.0f},{220.0f,42.0f},app.player.oxygen,imguiColor({0.035f,0.055f,0.075f},0.82f),imguiColor({0.30f,0.72f,1.0f}));if(app.rain)d->AddText({24.0f,52.0f},imguiColor(app.playerSheltered?Vec3{0.72f,0.86f,0.76f}:Vec3{0.65f,0.76f,0.92f}),app.playerSheltered?"SHELTER":"RAIN");if(app.chunks&&!app.inventoryOpen&&!app.debugOpen){RayHit hit=raycast(*app.chunks,playerEye(app),cameraForward(app.camera));if(hit.hit){std::string text=blockName(hit.type);ImVec2 ts=ImGui::CalcTextSize(text.c_str());ImVec2 p{fw*0.5f-ts.x*0.5f,fh*0.5f+22.0f};d->AddRectFilled({p.x-10.0f,p.y-6.0f},{p.x+ts.x+10.0f,p.y+ts.y+7.0f},imguiColor({0.02f,0.025f,0.03f},0.58f),6.0f);d->AddText(p,imguiColor({0.96f,0.88f,0.62f}),text.c_str());}}}
static void drawTargetHighlight(App& app,int w,int h){if(!app.chunks||app.inventoryOpen||app.debugOpen)return;RayHit hit=raycast(*app.chunks,playerEye(app),cameraForward(app.camera));if(!hit.hit)return;float aspect=static_cast<float>(w)/std::max(static_cast<float>(h),1.0f);Mat4 proj=perspective(65.0f*Pi/180.0f,aspect,0.1f,420.0f);Vec3 forward=cameraForward(app.camera);Mat4 view=lookAt(app.camera.p,app.camera.p+forward,{0.0f,1.0f,0.0f});drawBlockHighlight(ImGui::GetForegroundDrawList(),hit,mul(proj,view),w,h);}
static void drawMesh(const Mesh& mesh,GLint uModel,GLint uMvp,GLint uTint,Mat4 projection,Mat4 view,Mat4 modelMatrix,Vec3 tint){if(!mesh.indices)return;Mat4 mvp=mul(projection,mul(view,modelMatrix));oglUniformMatrix4fv(uModel,1,GL_FALSE,modelMatrix.m);oglUniformMatrix4fv(uMvp,1,GL_FALSE,mvp.m);oglUniform3f(uTint,tint.x,tint.y,tint.z);oglBindVertexArray(mesh.vao);glDrawElements(GL_TRIANGLES,mesh.indices,GL_UNSIGNED_INT,nullptr);}
static void drawAvatarPart(const Mesh& cube,GLint uModel,GLint uMvp,GLint uTint,Mat4 proj,Mat4 view,Vec3 base,float yaw,Vec3 local,Vec3 size,Vec3 tint,float rx=0.0f,float rz=0.0f){Mat4 model=mul(translate(base),mul(rotateY(yaw),mul(translate(local),mul(rotateZ(rz),mul(rotateX(rx),scaleM(size))))));drawMesh(cube,uModel,uMvp,uTint,proj,view,model,tint);}
static void drawAvatarPart3(const Mesh& cube,GLint uModel,GLint uMvp,GLint uTint,Mat4 proj,Mat4 view,Vec3 base,float yaw,Vec3 local,Vec3 size,Vec3 tint,float rx=0.0f,float ry=0.0f,float rz=0.0f){Mat4 model=mul(translate(base),mul(rotateY(yaw),mul(translate(local),mul(rotateY(ry),mul(rotateZ(rz),mul(rotateX(rx),scaleM(size)))))));drawMesh(cube,uModel,uMvp,uTint,proj,view,model,tint);}
static void drawAvatarChainPart(const Mesh& cube,GLint uModel,GLint uMvp,GLint uTint,Mat4 proj,Mat4 view,Vec3 base,float yaw,Vec3 pivot,Vec3 joint,Vec3 local,Vec3 size,Vec3 tint,float rx1,float ry1,float rz1,float rx2=0.0f,float ry2=0.0f,float rz2=0.0f){Mat4 model=translate(base);model=mul(model,rotateY(yaw));model=mul(model,translate(pivot));model=mul(model,rotateY(ry1));model=mul(model,rotateZ(rz1));model=mul(model,rotateX(rx1));model=mul(model,translate(joint));model=mul(model,rotateY(ry2));model=mul(model,rotateZ(rz2));model=mul(model,rotateX(rx2));model=mul(model,translate(local));model=mul(model,scaleM(size));drawMesh(cube,uModel,uMvp,uTint,proj,view,model,tint);}
static void drawFoxArms(const App& app,const Mesh& cube,GLint uModel,GLint uMvp,GLint uTint,Mat4 proj,Mat4 view,Vec3 base,float yaw,float arm,float swim,float crouch,float air,float run,Vec3 fur,Vec3 fur2,Vec3 dark,Vec3 white){float t=static_cast<float>(glfwGetTime());float mine=app.avatarAnim?std::clamp(app.breakProgress*1.35f,0.0f,1.0f):0.0f;float hit=std::sin(t*18.0f)*0.38f*mine;float left=arm+swim*0.30f+crouch*0.18f-air*0.10f;float right=-arm+swim*0.30f+crouch*0.18f+air*0.18f-mine*(1.18f+hit);float leftBend=0.30f+std::abs(arm)*0.28f+swim*0.42f+crouch*0.16f;float rightBend=0.30f+std::abs(arm)*0.28f+swim*0.42f+crouch*0.16f+mine*(0.92f-hit*0.25f);float leftOut=-0.13f-run*0.06f+swim*0.06f;float rightOut=0.13f+run*0.06f-swim*0.06f;float leftYaw=-swim*0.16f;float rightYaw=swim*0.16f-mine*0.28f;Vec3 lp{-0.39f,1.27f-crouch*0.04f,-0.02f};Vec3 rp{0.39f,1.27f-crouch*0.04f,-0.02f};drawAvatarChainPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,lp,{0.0f,0.0f,0.0f},{0.0f,-0.22f,0.0f},{0.19f,0.43f,0.20f},fur,left,leftYaw,leftOut);drawAvatarChainPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,rp,{0.0f,0.0f,0.0f},{0.0f,-0.22f,0.0f},{0.19f,0.43f,0.20f},fur,right,rightYaw,rightOut);drawAvatarChainPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,lp,{0.0f,-0.43f,0.0f},{0.0f,-0.19f,-0.03f},{0.17f,0.36f,0.18f},fur2,left,leftYaw,leftOut,leftBend);drawAvatarChainPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,rp,{0.0f,-0.43f,0.0f},{0.0f,-0.19f,-0.03f},{0.17f,0.36f,0.18f},fur2,right,rightYaw,rightOut,rightBend);drawAvatarChainPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,lp,{0.0f,-0.43f,0.0f},{0.0f,-0.39f,-0.08f},{0.21f,0.15f,0.23f},dark,left,leftYaw,leftOut,leftBend*1.10f);drawAvatarChainPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,rp,{0.0f,-0.43f,0.0f},{0.0f,-0.39f,-0.08f},{0.21f,0.15f,0.23f},dark,right,rightYaw,rightOut,rightBend*1.10f);drawAvatarChainPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,lp,{0.0f,-0.43f,0.0f},{-0.055f,-0.49f,-0.18f},{0.035f,0.030f,0.085f},white,left,leftYaw,leftOut,leftBend*1.10f);drawAvatarChainPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,lp,{0.0f,-0.43f,0.0f},{0.055f,-0.49f,-0.18f},{0.035f,0.030f,0.085f},white,left,leftYaw,leftOut,leftBend*1.10f);drawAvatarChainPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,rp,{0.0f,-0.43f,0.0f},{-0.055f,-0.49f,-0.18f},{0.035f,0.030f,0.085f},white,right,rightYaw,rightOut,rightBend*1.10f);drawAvatarChainPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,rp,{0.0f,-0.43f,0.0f},{0.055f,-0.49f,-0.18f},{0.035f,0.030f,0.085f},white,right,rightYaw,rightOut,rightBend*1.10f);}
static void drawFoxTail(const App& app,const Mesh& cube,GLint uModel,GLint uMvp,GLint uTint,Mat4 proj,Mat4 view,Vec3 base,float yaw){Vec3 fur{0.90f,0.33f,0.08f};Vec3 shade{0.72f,0.20f,0.055f};Vec3 tip{0.96f,0.90f,0.76f};float x=0.0f;for(int i=0;i<7;i++){float k=static_cast<float>(i);float a=app.tailYaw[i];x+=std::sin(a)*(0.08f+k*0.011f);float y=0.93f+k*0.030f-k*k*0.006f;float z=0.45f+k*0.185f;float s=1.0f-k*0.055f;Vec3 c=i>4?tip:fur;drawAvatarPart3(cube,uModel,uMvp,uTint,proj,view,base,yaw,{x,y,z},{0.35f*s,0.31f*s,0.35f},c,app.tailPitch[i],a,a*0.18f);if(i<6){drawAvatarPart3(cube,uModel,uMvp,uTint,proj,view,base,yaw,{x-0.17f*s,y-0.015f,z-0.015f},{0.14f*s,0.22f*s,0.24f},i>4?tip:shade,app.tailPitch[i],a+0.18f,a*0.10f);drawAvatarPart3(cube,uModel,uMvp,uTint,proj,view,base,yaw,{x+0.17f*s,y-0.015f,z-0.015f},{0.14f*s,0.22f*s,0.24f},i>4?tip:shade,app.tailPitch[i],a-0.18f,a*0.10f);}}}
static void drawFoxAvatar(const App& app,const Mesh& cube,GLint uModel,GLint uMvp,GLint uTint,Mat4 proj,Mat4 view){float t=static_cast<float>(glfwGetTime());float speed=len({app.player.v.x,0.0f,app.player.v.z});float walk=std::clamp(speed/7.0f,0.0f,1.0f);float run=std::clamp((speed-5.4f)/5.0f,0.0f,1.0f);float swim=app.player.swimming?1.0f:0.0f;float crouch=app.player.crouching?1.0f:0.0f;float air=(!app.player.grounded&&!app.player.swimming&&!app.fly)?1.0f:0.0f;float idle=(1.0f-walk)*(1.0f-swim)*(1.0f-air);float bob=app.avatarAnim?(std::sin(t*8.8f)*walk*0.035f+std::sin(t*1.8f)*idle*0.015f+std::sin(t*5.0f)*swim*0.025f):0.0f;Vec3 base=app.player.p+Vec3{0.0f,bob-crouch*0.22f+swim*0.06f,0.0f};float yaw=-(app.camera.yaw*Pi/180.0f)-Pi*0.5f;float arm=app.avatarAnim?std::sin(t*(8.8f+run*3.5f))*walk*(0.62f+run*0.22f)+std::sin(t*5.5f)*swim*0.55f+air*0.35f:0.0f;float leg=app.avatarAnim?-std::sin(t*(8.8f+run*3.5f))*walk*(0.62f+run*0.26f)+std::sin(t*5.5f+Pi)*swim*0.45f-air*0.25f:0.0f;float ear=app.avatarAnim?(std::sin(t*3.2f)*0.045f+std::sin(t*10.0f)*idle*0.020f):0.0f;float bodyPitch=swim*0.54f+crouch*0.18f-air*0.12f;float headPitch=-swim*0.20f+crouch*0.10f+air*0.06f;Vec3 fur{0.90f,0.33f,0.08f};Vec3 fur2{0.70f,0.20f,0.055f};Vec3 dark{0.055f,0.038f,0.030f};Vec3 white{0.96f,0.90f,0.76f};Vec3 eye{0.02f,0.10f,0.08f};drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.0f,0.93f,0.03f},{0.58f,0.76f-crouch*0.10f,0.42f},fur,bodyPitch);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.0f,0.94f,0.265f},{0.34f,0.58f,0.055f},fur2,bodyPitch*0.8f);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.0f,0.98f,-0.245f},{0.36f,0.53f,0.055f},white,bodyPitch*0.8f);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.0f,0.55f-crouch*0.08f,0.02f},{0.50f,0.35f,0.40f},fur2,bodyPitch*0.45f);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.0f,1.34f-crouch*0.05f,0.02f},{0.52f,0.16f,0.38f},fur2,bodyPitch*0.65f);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.0f,1.55f-crouch*0.05f,-0.34f},{0.48f,0.43f,0.44f},fur,headPitch);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.0f,1.71f-crouch*0.05f,-0.34f},{0.28f,0.06f,0.28f},fur2,headPitch);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{-0.26f,1.50f,-0.50f},{0.13f,0.19f,0.08f},white,headPitch);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.26f,1.50f,-0.50f},{0.13f,0.19f,0.08f},white,headPitch);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{-0.12f,1.57f,-0.585f},{0.08f,0.07f,0.035f},eye,headPitch);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.12f,1.57f,-0.585f},{0.08f,0.07f,0.035f},eye,headPitch);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{-0.12f,1.59f,-0.610f},{0.032f,0.030f,0.018f},{0.62f,0.92f,0.62f},headPitch);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.12f,1.59f,-0.610f},{0.032f,0.030f,0.018f},{0.62f,0.92f,0.62f},headPitch);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.0f,1.43f,-0.70f},{0.31f,0.19f,0.28f},white,headPitch);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.0f,1.47f,-0.90f},{0.13f,0.08f,0.08f},dark,headPitch);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.0f,1.36f,-0.80f},{0.20f,0.05f,0.06f},dark,headPitch);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{-0.21f,1.84f,-0.38f},{0.17f,0.33f,0.15f},fur,headPitch*0.6f,-0.20f+ear);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.21f,1.84f,-0.38f},{0.17f,0.33f,0.15f},fur,headPitch*0.6f,0.20f-ear);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{-0.21f,1.88f,-0.43f},{0.09f,0.20f,0.055f},white,headPitch*0.6f,-0.20f+ear);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.21f,1.88f,-0.43f},{0.09f,0.20f,0.055f},white,headPitch*0.6f,0.20f-ear);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{-0.21f,2.03f,-0.40f},{0.10f,0.10f,0.10f},dark);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.21f,2.03f,-0.40f},{0.10f,0.10f,0.10f},dark);drawFoxArms(app,cube,uModel,uMvp,uTint,proj,view,base,yaw,arm,swim,crouch,air,run,fur,fur2,dark,white);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{-0.16f,0.34f-crouch*0.06f,-0.02f},{0.20f,0.52f-crouch*0.10f,0.22f},fur,leg+crouch*0.38f+swim*0.26f);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.16f,0.34f-crouch*0.06f,-0.02f},{0.20f,0.52f-crouch*0.10f,0.22f},fur,-leg+crouch*0.38f+swim*0.26f);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{-0.16f,0.07f,-0.16f},{0.24f,0.13f,0.34f},dark,crouch*0.20f+swim*0.32f);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.16f,0.07f,-0.16f},{0.24f,0.13f,0.34f},dark,crouch*0.20f+swim*0.32f);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{-0.27f,0.03f,-0.30f},{0.045f,0.035f,0.10f},white);drawAvatarPart(cube,uModel,uMvp,uTint,proj,view,base,yaw,{0.27f,0.03f,-0.30f},{0.045f,0.035f,0.10f},white);drawFoxTail(app,cube,uModel,uMvp,uTint,proj,view,base,yaw);}
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
if(aType>5.5&&aType<6.5&&aNormal.y>0.5)pos.y+=sin(pos.x*0.65+pos.z*0.48+uTime*1.7)*0.070+sin(pos.x*1.35-pos.z*0.78+uTime*1.1)*0.036+sin((pos.x+pos.z)*2.4+uTime*2.0)*0.014;
if(aType>8.5&&aType<9.5&&fract(pos.y)>0.05)pos.xz+=vec2(sin(uTime*1.7+pos.z*1.4),cos(uTime*1.2+pos.x*1.1))*0.035;
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
uniform float uFogDistance;
uniform int uUnderWater;
uniform float uRain;
uniform vec3 uBreakBlock;
uniform float uBreakProgress;
uniform sampler2D uShadowMap;
out vec4 FragColor;
float hash(vec3 p){
return fract(sin(dot(p,vec3(12.9898,78.233,37.719)))*43758.5453);
}
float noise3(vec3 p){
vec3 i=floor(p);
vec3 f=fract(p);
f=f*f*(3.0-2.0*f);
float a=hash(i);
float b=hash(i+vec3(1.0,0.0,0.0));
float c=hash(i+vec3(0.0,1.0,0.0));
float d=hash(i+vec3(1.0,1.0,0.0));
float e=hash(i+vec3(0.0,0.0,1.0));
float g=hash(i+vec3(1.0,0.0,1.0));
float j=hash(i+vec3(0.0,1.0,1.0));
float k=hash(i+vec3(1.0,1.0,1.0));
return mix(mix(mix(a,b,f.x),mix(c,d,f.x),f.y),mix(mix(e,g,f.x),mix(j,k,f.x),f.y),f.z);
}
float fbm3(vec3 p){
float v=0.0;
float a=0.52;
for(int i=0;i<5;i++){v+=noise3(p)*a;p=p*2.03+vec3(7.1,3.7,5.9);a*=0.50;}
return v;
}
vec3 textureColor(vec3 color,float type,vec3 p,vec3 n){
float broad=fbm3(p*0.42+n*1.7);
float fine=fbm3(p*2.25+n*3.1);
float micro=fbm3(p*7.2+n*5.3);
float grain=0.84+broad*0.22+fine*0.10;
float veins=sin(dot(p,vec3(1.13,0.74,0.91))*3.8+fbm3(p*0.86)*4.2);
float top=smoothstep(0.16,0.82,n.y);
if(type>0.5&&type<1.5){vec3 topColor=mix(vec3(0.14,0.38,0.12),vec3(0.26,0.70,0.20),broad);vec3 sideColor=mix(vec3(0.28,0.18,0.10),vec3(0.38,0.26,0.14),fine);color=mix(sideColor,topColor,top)*(0.88+micro*0.16);}
if(type>1.5&&type<2.5){vec3 a=vec3(0.30,0.19,0.11);vec3 b=vec3(0.48,0.32,0.18);color=mix(a,b,broad)*(0.88+fine*0.18);}
if(type>2.5&&type<3.5){vec3 stone=mix(vec3(0.30,0.31,0.31),vec3(0.55,0.55,0.52),broad);color=stone*(0.86+fine*0.20)+vec3(0.06)*smoothstep(0.70,0.98,abs(veins));}
if(type>3.5&&type<4.5){color=mix(vec3(0.80,0.86,0.88),vec3(1.0,1.0,0.96),top)*(0.92+fine*0.08);}
if(type>4.5&&type<5.5){float rings=0.5+0.5*sin((p.y+p.x*0.17+p.z*0.15)*7.8+fbm3(p*0.7)*2.0);color=mix(vec3(0.30,0.16,0.07),vec3(0.64,0.40,0.18),rings)*(0.86+fine*0.18);}
if(type>5.5&&type<6.5){float wave=sin(p.x*1.2+uTime*1.15)+sin(p.z*1.7-uTime*0.95)+sin((p.x+p.z)*2.2-uTime*1.45)*0.45;color=mix(vec3(0.04,0.30,0.62),vec3(0.12,0.55,0.88),0.55+broad*0.20+wave*0.035);}
if(type>6.5&&type<7.5){vec3 leaf=mix(vec3(0.08,0.30,0.08),vec3(0.22,0.58,0.15),broad);color=leaf*(0.84+fine*0.20);}
if(type>7.5&&type<8.5){vec3 sand=mix(vec3(0.60,0.52,0.32),vec3(0.86,0.76,0.48),broad);float dune=sin((p.x+p.z)*2.2+fbm3(p*0.55)*3.0)*0.035;color=sand*(0.94+fine*0.10+dune);}
if(type>8.5&&type<9.5){color=mix(vec3(0.12,0.42,0.08),vec3(0.34,0.86,0.20),broad)*(0.84+fine*0.22);}
if(type>9.5&&type<10.5)color=mix(vec3(0.30,0.31,0.31),vec3(0.035,0.035,0.032),smoothstep(0.54,0.86,broad));
if(type>10.5&&type<11.5)color=mix(vec3(0.34,0.32,0.30),vec3(0.82,0.58,0.40),smoothstep(0.52,0.84,broad));
if(type>11.5&&type<12.5)color=mix(vec3(0.34,0.31,0.27),vec3(1.0,0.72,0.18),smoothstep(0.50,0.82,broad));
if(type>12.5&&type<13.5){float board=0.5+0.5*sin(p.y*11.0+fbm3(p*0.9)*1.8);color=mix(vec3(0.43,0.23,0.10),vec3(0.78,0.49,0.22),board)*(0.88+fine*0.14);}
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
float water=step(5.5,vType)*step(vType,6.5);
if(water>0.5){vec2 wv=vec2(sin(vPos.x*1.8+uTime*1.3)+sin((vPos.x+vPos.z)*1.1-uTime*0.8),cos(vPos.z*1.6-uTime*1.1)+sin((vPos.x-vPos.z)*1.4+uTime*0.9));n=normalize(n+vec3(wv.x*0.10,0.0,wv.y*0.10)*max(n.y,0.0));}
if(water<0.5){vec3 q=vPos*1.7;float b=fbm3(q);vec3 g=vec3(fbm3(q+vec3(0.08,0.0,0.0))-b,fbm3(q+vec3(0.0,0.08,0.0))-b,fbm3(q+vec3(0.0,0.0,0.08))-b);n=normalize(n-g*(0.14+0.12*abs(n.y)));}
vec3 viewDir=normalize(uCamera-vPos);
float diffuse=max(dot(n,normalize(uSunDir)),0.0);
float shadow=mix(shadowFactor(n),1.0,water*0.72);
float lightLevel=clamp((max(max(uSunColor.r,uSunColor.g),uSunColor.b)-0.10)/0.90,0.0,1.0);
float wrap=diffuse*shadow*(0.44+lightLevel*0.44)+(0.045+lightLevel*0.19);
float spec=pow(max(dot(reflect(-normalize(uSunDir),n),viewDir),0.0),mix(36.0,90.0,water))*mix(0.08,0.42,water)*shadow;
float fog=clamp(length(vPos-uCamera)/max(uFogDistance,1.0),0.0,1.0);
float underwater=float(uUnderWater);
fog=mix(fog,clamp(length(vPos-uCamera)/90.0,0.0,1.0),underwater*0.65);
float sideShadow=mix(0.72,1.0,abs(n.y));
float edge=min(min(fract(vPos.x),1.0-fract(vPos.x)),min(min(fract(vPos.y),1.0-fract(vPos.y)),min(fract(vPos.z),1.0-fract(vPos.z))));
float contact=0.95+fbm3(vPos*0.70)*0.06-(1.0-smoothstep(0.00,0.055,edge))*0.035;
vec3 base=textureColor(vColor*uTint,vType,vPos,n);
vec3 color=base*wrap*sideShadow*contact+uSunColor*spec;
float night=1.0-lightLevel;
float shimmer=sin(vPos.x*2.5+uTime*1.8)*sin(vPos.z*1.9-uTime*1.4);
float fresnel=pow(1.0-max(dot(viewDir,n),0.0),2.0);
color=mix(color,vec3(0.15,0.52,0.88),water*(0.30+shimmer*0.05+fresnel*0.10));
color=mix(color,color*0.78+vec3(0.025,0.045,0.060),uRain*(1.0-water)*0.30);
color+=vec3(0.06,0.09,0.11)*uRain*water;
color=mix(color,color*0.36+vec3(0.012,0.020,0.045),night*0.70);
vec3 cell=floor(vPos+vec3(0.002));
float target=1.0-step(0.20,length(cell-uBreakBlock));
float crackA=abs(fract((vPos.x+vPos.y*0.37+vPos.z*0.19)*7.0+hash(cell))-0.5);
float crackB=abs(fract((vPos.z-vPos.y*0.41+vPos.x*0.23)*8.5+hash(cell+3.0))-0.5);
float crack=1.0-smoothstep(0.0,0.018+uBreakProgress*0.080,min(crackA,crackB));
color=mix(color,color*0.22+vec3(0.02,0.018,0.014),target*crack*step(0.01,uBreakProgress));
color+=vec3(0.90,0.66,0.32)*target*uBreakProgress*0.035;
color=mix(color,vec3(0.04,0.22,0.42),underwater*(0.20+fog*0.24));
float alpha=mix(1.0,0.50+fresnel*0.20,water);
vec3 fogColor=mix(vec3(0.008,0.014,0.030),uSkyColor,lightLevel);
vec3 outColor=mix(color,fogColor,fog);
outColor=clamp((outColor*(2.51*outColor+0.03))/(outColor*(2.43*outColor+0.59)+0.14),0.0,1.0);
outColor=pow(outColor,vec3(0.92));
FragColor=vec4(outColor,alpha);
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
uniform int uClouds;
uniform float uRain;
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
vec3 sunDir=normalize(uSunDir);
float daylight=clamp((sunDir.y+0.22)*1.25,0.02,1.0);
vec3 horizon=mix(vec3(0.010,0.014,0.035),vec3(0.78,0.84,0.92),daylight);
vec3 zenith=mix(vec3(0.004,0.008,0.026),vec3(0.18,0.42,0.82),daylight);
float sun=max(dot(dir,sunDir),0.0);
float disk=smoothstep(0.997,0.9995,sun);
float glow=pow(sun,32.0)*0.42+pow(sun,384.0)*0.72;
vec3 moonDir=normalize(-sunDir+vec3(0.0,0.08,0.0));
float moon=max(dot(dir,moonDir),0.0);
float moonDisk=smoothstep(0.9965,0.9994,moon);
float moonGlow=pow(moon,42.0)*0.22;
float cloudMask=smoothstep(0.03,0.42,dir.y);
vec2 cloudUv=dir.xz/max(dir.y+0.28,0.18)*0.46+vec2(uTime*0.006,uTime*0.002);
float cloudBase=fbm(cloudUv*1.10)*0.64+fbm(cloudUv*2.70+20.0)*0.26+fbm(cloudUv*7.50-13.0)*0.10;
float cloud=smoothstep(0.48,0.78,cloudBase)*cloudMask*float(uClouds);
float veil=smoothstep(0.34,0.72,fbm(cloudUv*0.62+44.0))*cloudMask*float(uClouds);
float cloudLight=clamp(dot(normalize(vec3(dir.x,0.25,dir.z)),sunDir)*0.5+0.5,0.0,1.0);
vec3 cloudColor=mix(vec3(0.72,0.76,0.80),vec3(1.0,0.94,0.82),cloudLight);
float rays=pow(sun,7.0)*0.34*(1.0-cloud*0.52)+pow(max(dot(dir,sunDir*vec3(1.0,0.55,1.0)),0.0),18.0)*0.10;
vec3 color=mix(horizon,zenith,h)+vec3(1.0,0.64,0.24)*glow+vec3(1.0,0.88,0.55)*disk;
color=mix(color,cloudColor*(0.35+daylight*0.65),cloud*0.72);
color=mix(color,cloudColor*(0.28+daylight*0.50),veil*0.18);
color+=vec3(1.0,0.72,0.34)*rays*daylight;
float stars=step(0.9965,h2(floor(dir.xz*520.0)))*smoothstep(0.04,0.34,dir.y)*(1.0-cloud);
float starMist=smoothstep(0.58,0.90,fbm(dir.xz*3.0+vec2(dir.y*2.1,17.0)))*smoothstep(0.02,0.42,dir.y)*(1.0-cloud);
color+=vec3(0.66,0.76,1.0)*stars*(1.0-daylight);
color+=vec3(0.28,0.36,0.70)*starMist*(1.0-daylight)*0.22;
color+=vec3(0.62,0.74,1.0)*(moonGlow+moonDisk*0.72)*(1.0-daylight)*(1.0-cloud*0.45);
color=mix(color,vec3(0.08,0.10,0.14),uRain*0.48);
color=clamp((color*(2.51*color+0.03))/(color*(2.43*color+0.59)+0.14),0.0,1.0);
color=pow(color,vec3(0.92));
FragColor=vec4(color,1.0);
})glsl";
int main(int argc,char** argv){try{bool smoke=argc>1&&std::string(argv[1])=="--smoke";if(!glfwInit())throw std::runtime_error("GLFW init failed");glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);if(smoke)glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
#if defined(__APPLE__)
glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GLFW_TRUE);
#endif
GLFWwindow* window=glfwCreateWindow(1280,720,"OpenGLWorld",nullptr,nullptr);if(!window)throw std::runtime_error("Window creation failed");glfwMakeContextCurrent(window);glfwSwapInterval(1);loadGl();App app;std::vector<Chunk> chunks;app.chunks=&chunks;app.player.p=findSpawn();ensureChunks(chunks,app.player.p,app.renderDistance);keepPlayerClear(app);updateCamera(app);rebuildDirtyChunks(chunks);glfwSetWindowUserPointer(window,&app);glfwSetCursorPosCallback(window,cursorCallback);glfwSetScrollCallback(window,scrollCallback);glfwSetMouseButtonCallback(window,mouseButtonCallback);glfwSetKeyCallback(window,keyCallback);glfwSetCharCallback(window,charCallback);glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);GLuint worldProgram=createProgram(worldVs,worldFs);GLuint shadowProgram=createProgram(shadowVs,shadowFs);GLuint skyProgram=createProgram(skyVs,skyFs);GLint uModel=oglGetUniformLocation(worldProgram,"uModel");GLint uMvp=oglGetUniformLocation(worldProgram,"uMvp");GLint uTint=oglGetUniformLocation(worldProgram,"uTint");GLint uSunDir=oglGetUniformLocation(worldProgram,"uSunDir");GLint uSunColor=oglGetUniformLocation(worldProgram,"uSunColor");GLint uSkyColor=oglGetUniformLocation(worldProgram,"uSkyColor");GLint uCamera=oglGetUniformLocation(worldProgram,"uCamera");GLint uTime=oglGetUniformLocation(worldProgram,"uTime");GLint uFogDistance=oglGetUniformLocation(worldProgram,"uFogDistance");GLint uUnderWater=oglGetUniformLocation(worldProgram,"uUnderWater");GLint uRain=oglGetUniformLocation(worldProgram,"uRain");GLint uLightMvp=oglGetUniformLocation(worldProgram,"uLightMvp");GLint uShadowMap=oglGetUniformLocation(worldProgram,"uShadowMap");GLint shLightMvp=oglGetUniformLocation(shadowProgram,"uLightMvp");GLint sForward=oglGetUniformLocation(skyProgram,"uForward");GLint sRight=oglGetUniformLocation(skyProgram,"uRight");GLint sUp=oglGetUniformLocation(skyProgram,"uUp");GLint sSunDir=oglGetUniformLocation(skyProgram,"uSunDir");GLint sTime=oglGetUniformLocation(skyProgram,"uTime");GLint sClouds=oglGetUniformLocation(skyProgram,"uClouds");GLint sRain=oglGetUniformLocation(skyProgram,"uRain");Mesh skyQuad=createScreenQuad();Mesh avatarCube=createAvatarCube();ShadowMap shadow=createShadowMap(2048);if(smoke){clearChunks(chunks);glfwTerminate();return 0;}IMGUI_CHECKVERSION();ImGui::CreateContext();ImGuiIO& io=ImGui::GetIO();io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard;ImGui::StyleColorsDark();ImGuiStyle& style=ImGui::GetStyle();style.WindowRounding=12.0f;style.FrameRounding=8.0f;style.PopupRounding=10.0f;style.WindowBorderSize=1.0f;style.FrameBorderSize=0.0f;style.Colors[ImGuiCol_WindowBg]=ImVec4(0.035f,0.042f,0.052f,0.92f);style.Colors[ImGuiCol_Border]=ImVec4(0.22f,0.28f,0.34f,0.86f);style.Colors[ImGuiCol_Text]=ImVec4(0.92f,0.91f,0.86f,1.0f);ImGui_ImplGlfw_InitForOpenGL(window,false);ImGui_ImplOpenGL3_Init("#version 330 core");Vec3 sunDir=norm({-0.38f,0.78f,-0.48f});Vec3 sunColor{1.0f,0.88f,0.62f};Vec3 skyColor{0.62f,0.76f,0.92f};double last=glfwGetTime();while(!glfwWindowShouldClose(window)){double now=glfwGetTime();float dt=std::min(static_cast<float>(now-last),0.033f);last=now;if(app.dayCycle)app.dayTime=fract(app.dayTime+dt*0.006f);float sunAngle=app.dayTime*2.0f*Pi;float sunY=std::sin(sunAngle);float dayLight=std::clamp(sunY*0.55f+0.48f,0.10f,1.0f);sunDir=norm({std::cos(sunAngle)*0.55f,std::max(sunY,-0.22f),-0.48f});sunColor={1.0f*dayLight,0.82f*dayLight,0.56f*dayLight};skyColor={0.04f+0.58f*dayLight,0.06f+0.70f*dayLight,0.12f+0.80f*dayLight};ensureChunks(chunks,app.player.p,app.renderDistance);glfwPollEvents();ImGui_ImplOpenGL3_NewFrame();ImGui_ImplGlfw_NewFrame();ImGui::NewFrame();updateInput(window,app,dt);ensureChunks(chunks,app.player.p,app.renderDistance);rebuildDirtyChunks(chunks,3);int w=0;int h=0;glfwGetFramebufferSize(window,&w,&h);float aspect=static_cast<float>(std::max(w,1))/static_cast<float>(std::max(h,1));Vec3 forward=cameraForward(app.camera);Vec3 right=norm(cross(forward,{0.0f,1.0f,0.0f}));Vec3 up=norm(cross(right,forward));Mat4 proj=perspective(65.0f*Pi/180.0f,aspect,0.1f,420.0f);Mat4 view=lookAt(app.camera.p,app.camera.p+forward,{0.0f,1.0f,0.0f});Mat4 lightView=lookAt(app.camera.p+sunDir*118.0f,app.camera.p,{0.0f,1.0f,0.0f});Mat4 lightProj=ortho(-112.0f,112.0f,-112.0f,112.0f,1.0f,250.0f);Mat4 lightMvp=mul(lightProj,lightView);oglBindFramebuffer(GL_FRAMEBUFFER,shadow.fbo);glViewport(0,0,shadow.size,shadow.size);glClear(GL_DEPTH_BUFFER_BIT);glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);glDisable(GL_BLEND);glDisable(GL_CULL_FACE);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);oglUseProgram(shadowProgram);oglUniformMatrix4fv(shLightMvp,1,GL_FALSE,lightMvp.m);for(const Chunk& c:chunks){if(c.mesh.indices){oglBindVertexArray(c.mesh.vao);glDrawElements(GL_TRIANGLES,c.mesh.indices,GL_UNSIGNED_INT,nullptr);}}oglBindFramebuffer(GL_FRAMEBUFFER,0);glViewport(0,0,w,h);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);oglUseProgram(skyProgram);oglUniform3f(sForward,forward.x,forward.y,forward.z);oglUniform3f(sRight,right.x*aspect*0.72f,right.y*aspect*0.72f,right.z*aspect*0.72f);oglUniform3f(sUp,up.x*0.72f,up.y*0.72f,up.z*0.72f);oglUniform3f(sSunDir,sunDir.x,sunDir.y,sunDir.z);oglUniform1f(sTime,static_cast<float>(now));oglUniform1i(sClouds,app.clouds?1:0);oglUniform1f(sRain,app.rain?1.0f:0.0f);oglBindVertexArray(skyQuad.vao);glDrawElements(GL_TRIANGLES,skyQuad.indices,GL_UNSIGNED_INT,nullptr);glEnable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glPolygonMode(GL_FRONT_AND_BACK,app.wireframe?GL_LINE:GL_FILL);oglUseProgram(worldProgram);oglUniform3f(uSunDir,sunDir.x,sunDir.y,sunDir.z);oglUniform3f(uSunColor,sunColor.x,sunColor.y,sunColor.z);oglUniform3f(uSkyColor,skyColor.x,skyColor.y,skyColor.z);oglUniform3f(uCamera,app.camera.p.x,app.camera.p.y,app.camera.p.z);oglUniform1f(uTime,static_cast<float>(now));oglUniform1f(uFogDistance,app.fogDistance);oglUniform1i(uUnderWater,cameraInWater(chunks,app.camera.p)?1:0);oglUniform1f(uRain,app.rain?1.0f:0.0f);oglUniform3f(oglGetUniformLocation(worldProgram,"uBreakBlock"),static_cast<float>(app.breakX),static_cast<float>(app.breakY),static_cast<float>(app.breakZ));oglUniform1f(oglGetUniformLocation(worldProgram,"uBreakProgress"),app.breakProgress);oglUniformMatrix4fv(uLightMvp,1,GL_FALSE,lightMvp.m);oglActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,shadow.texture);oglUniform1i(uShadowMap,0);for(const Chunk& c:chunks)drawMesh(c.mesh,uModel,uMvp,uTint,proj,view,identity(),{1.0f,1.0f,1.0f});if(app.thirdPerson)drawFoxAvatar(app,avatarCube,uModel,uMvp,uTint,proj,view);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDepthMask(GL_FALSE);for(const Chunk& c:chunks)drawMesh(c.waterMesh,uModel,uMvp,uTint,proj,view,identity(),{1.0f,1.0f,1.0f});glDepthMask(GL_TRUE);glDisable(GL_BLEND);drawImGuiHud(app,w,h);drawExtraHud(app,w,h);drawTargetHighlight(app,w,h);ImGui::Render();ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());glfwSwapBuffers(window);}ImGui_ImplOpenGL3_Shutdown();ImGui_ImplGlfw_Shutdown();ImGui::DestroyContext();destroyMesh(avatarCube);destroyMesh(skyQuad);clearChunks(chunks);glfwTerminate();return 0;}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;glfwTerminate();return 1;}}
