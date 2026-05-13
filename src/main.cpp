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
struct Vec3{float x;float y;float z;};
struct Mat4{float m[16];};
struct Vertex{Vec3 p;Vec3 n;Vec3 c;};
struct UiVertex{float x;float y;Vec3 c;};
struct Mesh{GLuint vao=0;GLuint vbo=0;GLuint ebo=0;GLsizei indices=0;};
struct UiMesh{GLuint vao=0;GLuint vbo=0;GLsizei vertices=0;};
struct Body{Vec3 p;Vec3 v;Vec3 color;float r;};
struct Pickup{Vec3 p;Vec3 color;int type;bool taken;float phase;};
struct Camera{Vec3 p{0.0f,12.0f,28.0f};float yaw=-90.0f;float pitch=-18.0f;double lastX=640.0;double lastY=360.0;bool firstMouse=true;};
struct App{Camera camera;bool paused=false;bool wireframe=false;bool lastPause=false;bool lastWire=false;bool lastReset=false;bool lastUse=false;int selected=0;int inventory[5]{0,0,0,0,0};bool lastSlot[5]{false,false,false,false,false};std::vector<Body>* bodies=nullptr;std::vector<Pickup>* pickups=nullptr;};
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
using GlBindVertexArray=void(APIENTRY*)(GLuint);
using GlGenBuffers=void(APIENTRY*)(GLsizei,GLuint*);
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
static GlBindVertexArray oglBindVertexArray=nullptr;
static GlGenBuffers oglGenBuffers=nullptr;
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
static Vec3& operator-=(Vec3& a,Vec3 b){a=a-b;return a;}
static float dot(Vec3 a,Vec3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
static Vec3 cross(Vec3 a,Vec3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
static float len(Vec3 v){return std::sqrt(dot(v,v));}
static Vec3 norm(Vec3 v){float l=len(v);return l>0.00001f?v/l:Vec3{0.0f,1.0f,0.0f};}
static Vec3 mix(Vec3 a,Vec3 b,float t){return a*(1.0f-t)+b*t;}
static float fract(float v){return v-std::floor(v);}
static float smooth(float t){return t*t*(3.0f-2.0f*t);}
static float rand2(int x,int z){return fract(std::sin(static_cast<float>(x)*127.1f+static_cast<float>(z)*311.7f)*43758.5453f)*2.0f-1.0f;}
static float noise(float x,float z){int xi=static_cast<int>(std::floor(x));int zi=static_cast<int>(std::floor(z));float xf=x-static_cast<float>(xi);float zf=z-static_cast<float>(zi);float u=smooth(xf);float v=smooth(zf);float a=rand2(xi,zi);float b=rand2(xi+1,zi);float c=rand2(xi,zi+1);float d=rand2(xi+1,zi+1);return (a*(1.0f-u)+b*u)*(1.0f-v)+(c*(1.0f-u)+d*u)*v;}
static float terrainHeight(float x,float z){float h=0.0f;float amp=5.5f;float freq=0.032f;for(int i=0;i<6;i++){h+=noise(x*freq,z*freq)*amp;amp*=0.48f;freq*=2.05f;}return h;}
static Vec3 terrainNormal(float x,float z){float e=0.35f;float l=terrainHeight(x-e,z);float r=terrainHeight(x+e,z);float d=terrainHeight(x,z-e);float u=terrainHeight(x,z+e);return norm({l-r,2.0f*e,d-u});}
static Vec3 terrainColor(float h,Vec3 n){float steep=1.0f-std::max(0.0f,n.y);Vec3 grass{0.22f,0.48f,0.18f};Vec3 dirt{0.38f,0.28f,0.16f};Vec3 rock{0.42f,0.42f,0.40f};Vec3 snow{0.86f,0.88f,0.84f};Vec3 c=mix(grass,dirt,std::clamp((h+1.0f)*0.13f,0.0f,1.0f));c=mix(c,rock,std::clamp(steep*2.2f,0.0f,1.0f));c=mix(c,snow,std::clamp((h-7.0f)*0.22f,0.0f,1.0f));return c;}
static Vec3 itemColor(int type){Vec3 colors[5]{{0.92f,0.35f,0.25f},{0.25f,0.72f,0.35f},{0.26f,0.55f,0.95f},{0.95f,0.78f,0.25f},{0.72f,0.38f,0.92f}};return colors[std::clamp(type,0,4)];}
static Mat4 identity(){Mat4 r{};r.m[0]=1.0f;r.m[5]=1.0f;r.m[10]=1.0f;r.m[15]=1.0f;return r;}
static Mat4 mul(Mat4 a,Mat4 b){Mat4 r{};for(int c=0;c<4;c++){for(int row=0;row<4;row++){r.m[c*4+row]=a.m[0*4+row]*b.m[c*4+0]+a.m[1*4+row]*b.m[c*4+1]+a.m[2*4+row]*b.m[c*4+2]+a.m[3*4+row]*b.m[c*4+3];}}return r;}
static Mat4 perspective(float fov,float aspect,float nearPlane,float farPlane){Mat4 r{};float t=std::tan(fov*0.5f);r.m[0]=1.0f/(aspect*t);r.m[5]=1.0f/t;r.m[10]=-(farPlane+nearPlane)/(farPlane-nearPlane);r.m[11]=-1.0f;r.m[14]=-(2.0f*farPlane*nearPlane)/(farPlane-nearPlane);return r;}
static Mat4 lookAt(Vec3 eye,Vec3 center,Vec3 up){Vec3 f=norm(center-eye);Vec3 s=norm(cross(f,up));Vec3 u=cross(s,f);Mat4 r=identity();r.m[0]=s.x;r.m[4]=s.y;r.m[8]=s.z;r.m[1]=u.x;r.m[5]=u.y;r.m[9]=u.z;r.m[2]=-f.x;r.m[6]=-f.y;r.m[10]=-f.z;r.m[12]=-dot(s,eye);r.m[13]=-dot(u,eye);r.m[14]=dot(f,eye);return r;}
static Mat4 model(Vec3 p,float s){Mat4 r=identity();r.m[0]=s;r.m[5]=s;r.m[10]=s;r.m[12]=p.x;r.m[13]=p.y;r.m[14]=p.z;return r;}
static Vec3 cameraForward(const Camera& c){float yaw=c.yaw*Pi/180.0f;float pitch=c.pitch*Pi/180.0f;return norm({std::cos(yaw)*std::cos(pitch),std::sin(pitch),std::sin(yaw)*std::cos(pitch)});}
template<class T>static T loadProc(const char* name){auto proc=glfwGetProcAddress(name);if(!proc)throw std::runtime_error(std::string("OpenGL function not found: ")+name);return reinterpret_cast<T>(proc);}
static void loadGl(){oglCreateShader=loadProc<GlCreateShader>("glCreateShader");oglShaderSource=loadProc<GlShaderSource>("glShaderSource");oglCompileShader=loadProc<GlCompileShader>("glCompileShader");oglGetShaderiv=loadProc<GlGetShaderiv>("glGetShaderiv");oglGetShaderInfoLog=loadProc<GlGetShaderInfoLog>("glGetShaderInfoLog");oglCreateProgram=loadProc<GlCreateProgram>("glCreateProgram");oglAttachShader=loadProc<GlAttachShader>("glAttachShader");oglLinkProgram=loadProc<GlLinkProgram>("glLinkProgram");oglGetProgramiv=loadProc<GlGetProgramiv>("glGetProgramiv");oglGetProgramInfoLog=loadProc<GlGetProgramInfoLog>("glGetProgramInfoLog");oglDeleteShader=loadProc<GlDeleteShader>("glDeleteShader");oglUseProgram=loadProc<GlUseProgram>("glUseProgram");oglGenVertexArrays=loadProc<GlGenVertexArrays>("glGenVertexArrays");oglBindVertexArray=loadProc<GlBindVertexArray>("glBindVertexArray");oglGenBuffers=loadProc<GlGenBuffers>("glGenBuffers");oglBindBuffer=loadProc<GlBindBuffer>("glBindBuffer");oglBufferData=loadProc<GlBufferData>("glBufferData");oglEnableVertexAttribArray=loadProc<GlEnableVertexAttribArray>("glEnableVertexAttribArray");oglVertexAttribPointer=loadProc<GlVertexAttribPointer>("glVertexAttribPointer");oglGetUniformLocation=loadProc<GlGetUniformLocation>("glGetUniformLocation");oglUniformMatrix4fv=loadProc<GlUniformMatrix4fv>("glUniformMatrix4fv");oglUniform3f=loadProc<GlUniform3f>("glUniform3f");}
static GLuint compileShader(GLenum type,const char* source){GLuint shader=oglCreateShader(type);oglShaderSource(shader,1,&source,nullptr);oglCompileShader(shader);GLint ok=0;oglGetShaderiv(shader,GL_COMPILE_STATUS,&ok);if(!ok){GLint size=0;oglGetShaderiv(shader,GL_INFO_LOG_LENGTH,&size);std::string log(static_cast<size_t>(std::max(size,1)),'\0');oglGetShaderInfoLog(shader,size,nullptr,log.data());throw std::runtime_error(log);}return shader;}
static GLuint createProgram(const char* vs,const char* fs){GLuint v=compileShader(GL_VERTEX_SHADER,vs);GLuint f=compileShader(GL_FRAGMENT_SHADER,fs);GLuint p=oglCreateProgram();oglAttachShader(p,v);oglAttachShader(p,f);oglLinkProgram(p);GLint ok=0;oglGetProgramiv(p,GL_LINK_STATUS,&ok);if(!ok){GLint size=0;oglGetProgramiv(p,GL_INFO_LOG_LENGTH,&size);std::string log(static_cast<size_t>(std::max(size,1)),'\0');oglGetProgramInfoLog(p,size,nullptr,log.data());throw std::runtime_error(log);}oglDeleteShader(v);oglDeleteShader(f);return p;}
static Mesh uploadMesh(const std::vector<Vertex>& vertices,const std::vector<unsigned int>& indices){Mesh mesh;mesh.indices=static_cast<GLsizei>(indices.size());oglGenVertexArrays(1,&mesh.vao);oglGenBuffers(1,&mesh.vbo);oglGenBuffers(1,&mesh.ebo);oglBindVertexArray(mesh.vao);oglBindBuffer(GL_ARRAY_BUFFER,mesh.vbo);oglBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size()*sizeof(Vertex)),vertices.data(),GL_STATIC_DRAW);oglBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh.ebo);oglBufferData(GL_ELEMENT_ARRAY_BUFFER,static_cast<GLsizeiptr>(indices.size()*sizeof(unsigned int)),indices.data(),GL_STATIC_DRAW);oglEnableVertexAttribArray(0);oglVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,p)));oglEnableVertexAttribArray(1);oglVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,n)));oglEnableVertexAttribArray(2);oglVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,c)));oglBindVertexArray(0);return mesh;}
static UiMesh createUiMesh(){UiMesh mesh;oglGenVertexArrays(1,&mesh.vao);oglGenBuffers(1,&mesh.vbo);oglBindVertexArray(mesh.vao);oglBindBuffer(GL_ARRAY_BUFFER,mesh.vbo);oglEnableVertexAttribArray(0);oglVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(UiVertex),reinterpret_cast<void*>(offsetof(UiVertex,x)));oglEnableVertexAttribArray(1);oglVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(UiVertex),reinterpret_cast<void*>(offsetof(UiVertex,c)));oglBindVertexArray(0);return mesh;}
static Mesh createTerrain(int grid,float step){std::vector<Vertex> vertices;std::vector<unsigned int> indices;vertices.reserve(static_cast<size_t>((grid+1)*(grid+1)));float half=static_cast<float>(grid)*step*0.5f;for(int z=0;z<=grid;z++){for(int x=0;x<=grid;x++){float wx=static_cast<float>(x)*step-half;float wz=static_cast<float>(z)*step-half;float h=terrainHeight(wx,wz);Vec3 n=terrainNormal(wx,wz);vertices.push_back({{wx,h,wz},n,terrainColor(h,n)});}}for(int z=0;z<grid;z++){for(int x=0;x<grid;x++){unsigned int a=static_cast<unsigned int>(z*(grid+1)+x);unsigned int b=a+1;unsigned int c=a+static_cast<unsigned int>(grid+1);unsigned int d=c+1;indices.push_back(a);indices.push_back(c);indices.push_back(b);indices.push_back(b);indices.push_back(c);indices.push_back(d);}}return uploadMesh(vertices,indices);}
static Mesh createSphere(int stacks,int slices){std::vector<Vertex> vertices;std::vector<unsigned int> indices;for(int i=0;i<=stacks;i++){float v=static_cast<float>(i)/static_cast<float>(stacks);float phi=v*Pi;float y=std::cos(phi);float r=std::sin(phi);for(int j=0;j<=slices;j++){float u=static_cast<float>(j)/static_cast<float>(slices);float theta=u*Pi*2.0f;Vec3 n{std::cos(theta)*r,y,std::sin(theta)*r};vertices.push_back({n,n,{1.0f,1.0f,1.0f}});}}for(int i=0;i<stacks;i++){for(int j=0;j<slices;j++){unsigned int a=static_cast<unsigned int>(i*(slices+1)+j);unsigned int b=a+1;unsigned int c=a+static_cast<unsigned int>(slices+1);unsigned int d=c+1;indices.push_back(a);indices.push_back(c);indices.push_back(b);indices.push_back(b);indices.push_back(c);indices.push_back(d);}}return uploadMesh(vertices,indices);}
static Mesh createScreenQuad(){std::vector<Vertex> vertices{{{-1.0f,-1.0f,0.0f},{0.0f,0.0f,1.0f},{1.0f,1.0f,1.0f}},{{1.0f,-1.0f,0.0f},{0.0f,0.0f,1.0f},{1.0f,1.0f,1.0f}},{{-1.0f,1.0f,0.0f},{0.0f,0.0f,1.0f},{1.0f,1.0f,1.0f}},{{1.0f,1.0f,0.0f},{0.0f,0.0f,1.0f},{1.0f,1.0f,1.0f}}};std::vector<unsigned int> indices{0,1,2,2,1,3};return uploadMesh(vertices,indices);}
static void resetBodies(std::vector<Body>& bodies){bodies.clear();for(int i=0;i<18;i++){float x=rand2(i*13,7)*45.0f;float z=rand2(3,i*17)*45.0f;float r=0.7f+fract(static_cast<float>(i)*0.37f)*0.85f;float y=terrainHeight(x,z)+7.0f+static_cast<float>(i%6)*2.2f;Vec3 color{0.25f+fract(static_cast<float>(i)*0.61f)*0.7f,0.28f+fract(static_cast<float>(i)*0.43f)*0.55f,0.3f+fract(static_cast<float>(i)*0.29f)*0.6f};Vec3 velocity{rand2(i,20)*3.0f,rand2(2,i)*1.5f,rand2(i,9)*3.0f};bodies.push_back({{x,y,z},velocity,color,r});}}
static void resetPickups(std::vector<Pickup>& pickups){pickups.clear();for(int i=0;i<36;i++){int type=i%5;float x=rand2(i*19,11)*54.0f;float z=rand2(5,i*23)*54.0f;float y=terrainHeight(x,z)+1.05f;pickups.push_back({{x,y,z},itemColor(type),type,false,fract(static_cast<float>(i)*0.41f)*Pi*2.0f});}}
static void stepPhysics(std::vector<Body>& bodies,float dt){float half=63.0f;float restitution=0.58f;for(Body& b:bodies){b.v.y-=18.0f*dt;b.p+=b.v*dt;float ground=terrainHeight(b.p.x,b.p.z)+b.r;if(b.p.y<ground){Vec3 n=terrainNormal(b.p.x,b.p.z);b.p.y=ground;float vn=dot(b.v,n);if(vn<0.0f)b.v-=n*((1.0f+restitution)*vn);b.v.x*=0.92f;b.v.z*=0.92f;}if(b.p.x<-half+b.r){b.p.x=-half+b.r;b.v.x=std::abs(b.v.x)*restitution;}if(b.p.x>half-b.r){b.p.x=half-b.r;b.v.x=-std::abs(b.v.x)*restitution;}if(b.p.z<-half+b.r){b.p.z=-half+b.r;b.v.z=std::abs(b.v.z)*restitution;}if(b.p.z>half-b.r){b.p.z=half-b.r;b.v.z=-std::abs(b.v.z)*restitution;}}for(size_t i=0;i<bodies.size();i++){for(size_t j=i+1;j<bodies.size();j++){Body& a=bodies[i];Body& b=bodies[j];Vec3 delta=b.p-a.p;float dist=len(delta);float target=a.r+b.r;if(dist>0.0001f&&dist<target){Vec3 n=delta/dist;float depth=target-dist;a.p-=n*(depth*0.5f);b.p+=n*(depth*0.5f);float rel=dot(b.v-a.v,n);if(rel<0.0f){float impulse=-(1.0f+0.72f)*rel*0.5f;a.v-=n*impulse;b.v+=n*impulse;}}}}}
static void collectPickups(std::vector<Pickup>& pickups,App& app){for(Pickup& p:pickups){if(!p.taken&&len(p.p-app.camera.p)<2.35f){p.taken=true;app.inventory[p.type]++;}}}
static void throwSelected(App& app){if(!app.bodies||app.inventory[app.selected]<=0)return;Vec3 f=cameraForward(app.camera);Vec3 pos=app.camera.p+f*2.2f;Vec3 vel=f*22.0f+Vec3{0.0f,2.8f,0.0f};app.bodies->push_back({pos,vel,itemColor(app.selected),0.48f});app.inventory[app.selected]--;}
static void cursorCallback(GLFWwindow* window,double xpos,double ypos){auto* app=static_cast<App*>(glfwGetWindowUserPointer(window));if(!app)return;Camera& c=app->camera;if(c.firstMouse){c.lastX=xpos;c.lastY=ypos;c.firstMouse=false;}float dx=static_cast<float>(xpos-c.lastX);float dy=static_cast<float>(c.lastY-ypos);c.lastX=xpos;c.lastY=ypos;float sens=0.09f;c.yaw+=dx*sens;c.pitch=std::clamp(c.pitch+dy*sens,-84.0f,84.0f);}
static bool pressed(GLFWwindow* window,int key){return glfwGetKey(window,key)==GLFW_PRESS;}
static void updateInput(GLFWwindow* window,App& app,float dt){if(pressed(window,GLFW_KEY_ESCAPE))glfwSetWindowShouldClose(window,GLFW_TRUE);bool pauseNow=pressed(window,GLFW_KEY_SPACE);if(pauseNow&&!app.lastPause)app.paused=!app.paused;app.lastPause=pauseNow;bool wireNow=pressed(window,GLFW_KEY_F);if(wireNow&&!app.lastWire)app.wireframe=!app.wireframe;app.lastWire=wireNow;bool resetNow=pressed(window,GLFW_KEY_R);if(resetNow&&!app.lastReset){if(app.bodies)resetBodies(*app.bodies);if(app.pickups)resetPickups(*app.pickups);}app.lastReset=resetNow;int keys[5]{GLFW_KEY_1,GLFW_KEY_2,GLFW_KEY_3,GLFW_KEY_4,GLFW_KEY_5};for(int i=0;i<5;i++){bool down=pressed(window,keys[i]);if(down&&!app.lastSlot[i])app.selected=i;app.lastSlot[i]=down;}bool useNow=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;if(useNow&&!app.lastUse)throwSelected(app);app.lastUse=useNow;Vec3 f=cameraForward(app.camera);Vec3 flat=norm({f.x,0.0f,f.z});Vec3 right=norm(cross(flat,{0.0f,1.0f,0.0f}));float speed=(pressed(window,GLFW_KEY_LEFT_SHIFT)||pressed(window,GLFW_KEY_RIGHT_SHIFT)?28.0f:12.0f)*dt;if(pressed(window,GLFW_KEY_W))app.camera.p+=flat*speed;if(pressed(window,GLFW_KEY_S))app.camera.p-=flat*speed;if(pressed(window,GLFW_KEY_D))app.camera.p+=right*speed;if(pressed(window,GLFW_KEY_A))app.camera.p-=right*speed;if(pressed(window,GLFW_KEY_E))app.camera.p.y+=speed;if(pressed(window,GLFW_KEY_Q))app.camera.p.y-=speed;float floorY=terrainHeight(app.camera.p.x,app.camera.p.z)+2.0f;if(app.camera.p.y<floorY)app.camera.p.y=floorY;}
static void pushRect(std::vector<UiVertex>& v,float x0,float y0,float x1,float y1,Vec3 c){v.push_back({x0,y0,c});v.push_back({x1,y0,c});v.push_back({x0,y1,c});v.push_back({x0,y1,c});v.push_back({x1,y0,c});v.push_back({x1,y1,c});}
static void uploadUi(UiMesh& mesh,const std::vector<UiVertex>& vertices){mesh.vertices=static_cast<GLsizei>(vertices.size());oglBindVertexArray(mesh.vao);oglBindBuffer(GL_ARRAY_BUFFER,mesh.vbo);oglBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size()*sizeof(UiVertex)),vertices.empty()?nullptr:vertices.data(),GL_DYNAMIC_DRAW);}
static void drawHotbar(UiMesh& mesh,GLuint program,const App& app,int w,int h){std::vector<UiVertex> vertices;float sy=0.15f;float sx=sy*static_cast<float>(std::max(h,1))/static_cast<float>(std::max(w,1));float gap=sx*0.18f;float total=sx*5.0f+gap*4.0f;float y0=-0.92f;float padY=sy*0.10f;float padX=padY*static_cast<float>(std::max(h,1))/static_cast<float>(std::max(w,1));float borderY=sy*0.035f;float borderX=borderY*static_cast<float>(std::max(h,1))/static_cast<float>(std::max(w,1));for(int i=0;i<5;i++){float x0=-total*0.5f+static_cast<float>(i)*(sx+gap);float x1=x0+sx;float y1=y0+sy;Vec3 bg=i==app.selected?Vec3{0.24f,0.25f,0.27f}:Vec3{0.11f,0.12f,0.14f};pushRect(vertices,x0,y0,x1,y1,bg);if(i==app.selected){Vec3 edge{0.95f,0.86f,0.42f};pushRect(vertices,x0-borderX,y0-borderY,x1+borderX,y0,edge);pushRect(vertices,x0-borderX,y1,x1+borderX,y1+borderY,edge);pushRect(vertices,x0-borderX,y0,x0,y1,edge);pushRect(vertices,x1,y0,x1+borderX,y1,edge);}Vec3 color=app.inventory[i]>0?itemColor(i):mix(itemColor(i),{0.04f,0.05f,0.06f},0.72f);pushRect(vertices,x0+padX,y0+padY,x1-padX,y1-padY,color);int bars=std::min(app.inventory[i],8);for(int b=0;b<bars;b++){float by0=y0+padY+static_cast<float>(b)*(sy-padY*2.0f)/8.0f;float by1=by0+(sy-padY*2.0f)/10.0f;pushRect(vertices,x1-padX*1.7f,by0,x1-padX*0.7f,by1,{0.96f,0.96f,0.88f});}}uploadUi(mesh,vertices);glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);oglUseProgram(program);oglBindVertexArray(mesh.vao);glDrawArrays(GL_TRIANGLES,0,mesh.vertices);}
static void drawMesh(const Mesh& mesh,GLint uModel,GLint uMvp,GLint uTint,Mat4 projection,Mat4 view,Mat4 modelMatrix,Vec3 tint){Mat4 mvp=mul(projection,mul(view,modelMatrix));oglUniformMatrix4fv(uModel,1,GL_FALSE,modelMatrix.m);oglUniformMatrix4fv(uMvp,1,GL_FALSE,mvp.m);oglUniform3f(uTint,tint.x,tint.y,tint.z);oglBindVertexArray(mesh.vao);glDrawElements(GL_TRIANGLES,mesh.indices,GL_UNSIGNED_INT,nullptr);}
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
float wrap=diffuse*0.82+0.24;
float spec=pow(max(dot(reflect(-normalize(uSunDir),n),viewDir),0.0),34.0)*0.22;
float heightLight=clamp((vPos.y+7.0)/22.0,0.0,1.0);
float fog=clamp(length(vPos-uCamera)/145.0,0.0,1.0);
vec3 color=vColor*uTint*wrap*mix(0.82,1.12,heightLight)+uSunColor*spec;
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
GLFWwindow* window=glfwCreateWindow(1280,720,"OpenGLWorld",nullptr,nullptr);if(!window)throw std::runtime_error("Window creation failed");glfwMakeContextCurrent(window);glfwSwapInterval(1);loadGl();App app;std::vector<Body> bodies;std::vector<Pickup> pickups;app.bodies=&bodies;app.pickups=&pickups;resetBodies(bodies);resetPickups(pickups);glfwSetWindowUserPointer(window,&app);glfwSetCursorPosCallback(window,cursorCallback);glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);GLuint worldProgram=createProgram(worldVs,worldFs);GLuint skyProgram=createProgram(skyVs,skyFs);GLuint uiProgram=createProgram(uiVs,uiFs);GLint uModel=oglGetUniformLocation(worldProgram,"uModel");GLint uMvp=oglGetUniformLocation(worldProgram,"uMvp");GLint uTint=oglGetUniformLocation(worldProgram,"uTint");GLint uSunDir=oglGetUniformLocation(worldProgram,"uSunDir");GLint uSunColor=oglGetUniformLocation(worldProgram,"uSunColor");GLint uSkyColor=oglGetUniformLocation(worldProgram,"uSkyColor");GLint uCamera=oglGetUniformLocation(worldProgram,"uCamera");GLint sForward=oglGetUniformLocation(skyProgram,"uForward");GLint sRight=oglGetUniformLocation(skyProgram,"uRight");GLint sUp=oglGetUniformLocation(skyProgram,"uUp");GLint sSunDir=oglGetUniformLocation(skyProgram,"uSunDir");Mesh terrain=createTerrain(128,1.0f);Mesh sphere=createSphere(18,28);Mesh skyQuad=createScreenQuad();UiMesh uiMesh=createUiMesh();if(smoke){glfwTerminate();return 0;}glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);Vec3 sunDir=norm({-0.38f,0.78f,-0.48f});Vec3 sunColor{1.0f,0.88f,0.62f};Vec3 skyColor{0.62f,0.76f,0.92f};double last=glfwGetTime();while(!glfwWindowShouldClose(window)){double now=glfwGetTime();float dt=std::min(static_cast<float>(now-last),0.033f);last=now;glfwPollEvents();updateInput(window,app,dt);collectPickups(pickups,app);if(!app.paused){int steps=3;for(int i=0;i<steps;i++)stepPhysics(bodies,dt/static_cast<float>(steps));}int w=0;int h=0;glfwGetFramebufferSize(window,&w,&h);float aspect=static_cast<float>(std::max(w,1))/static_cast<float>(std::max(h,1));Vec3 forward=cameraForward(app.camera);Vec3 right=norm(cross(forward,{0.0f,1.0f,0.0f}));Vec3 up=norm(cross(right,forward));Mat4 proj=perspective(65.0f*Pi/180.0f,aspect,0.1f,300.0f);Mat4 view=lookAt(app.camera.p,app.camera.p+forward,{0.0f,1.0f,0.0f});glViewport(0,0,w,h);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);oglUseProgram(skyProgram);oglUniform3f(sForward,forward.x,forward.y,forward.z);oglUniform3f(sRight,right.x*aspect*0.72f,right.y*aspect*0.72f,right.z*aspect*0.72f);oglUniform3f(sUp,up.x*0.72f,up.y*0.72f,up.z*0.72f);oglUniform3f(sSunDir,sunDir.x,sunDir.y,sunDir.z);oglBindVertexArray(skyQuad.vao);glDrawElements(GL_TRIANGLES,skyQuad.indices,GL_UNSIGNED_INT,nullptr);glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);glPolygonMode(GL_FRONT_AND_BACK,app.wireframe?GL_LINE:GL_FILL);oglUseProgram(worldProgram);oglUniform3f(uSunDir,sunDir.x,sunDir.y,sunDir.z);oglUniform3f(uSunColor,sunColor.x,sunColor.y,sunColor.z);oglUniform3f(uSkyColor,skyColor.x,skyColor.y,skyColor.z);oglUniform3f(uCamera,app.camera.p.x,app.camera.p.y,app.camera.p.z);drawMesh(terrain,uModel,uMvp,uTint,proj,view,identity(),{1.0f,1.0f,1.0f});for(const Pickup& p:pickups){if(!p.taken){Vec3 pos=p.p;pos.y+=std::sin(static_cast<float>(now)*2.4f+p.phase)*0.22f;drawMesh(sphere,uModel,uMvp,uTint,proj,view,model(pos,0.46f),p.color);}}for(const Body& b:bodies)drawMesh(sphere,uModel,uMvp,uTint,proj,view,model(b.p,b.r),b.color);drawHotbar(uiMesh,uiProgram,app,w,h);glfwSwapBuffers(window);}glfwTerminate();return 0;}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;glfwTerminate();return 1;}}
