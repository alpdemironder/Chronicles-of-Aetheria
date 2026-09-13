#include "GLHeaders.hpp"
#include <iostream>

namespace Aetheria {

PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLBUFFERSUBDATAPROC glBufferSubData = nullptr;

PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;

PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;

PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;

PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLUNIFORM1IPROC glUniform1i = nullptr;
PFNGLUNIFORM1FPROC glUniform1f = nullptr;
PFNGLUNIFORM2FPROC glUniform2f = nullptr;
PFNGLUNIFORM3FPROC glUniform3f = nullptr;
PFNGLUNIFORM4FPROC glUniform4f = nullptr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;

PFNGLTEXIMAGE3DPROC glTexImage3D = nullptr;
PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D = nullptr;
PFNGLACTIVETEXTUREPROC glActiveTexture = nullptr;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = nullptr;

PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = nullptr;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = nullptr;
PFNGLDRAWBUFFERSPROC glDrawBuffers = nullptr;

PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

static void* loadGLProc(const char* name) {
    void* p = (void*)wglGetProcAddress(name);
    if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
        HMODULE module = LoadLibraryA("opengl32.dll");
        if (module) {
            p = (void*)GetProcAddress(module, name);
        }
    }
    return p;
}

bool initGLFunctions() {
    glGenBuffers = (PFNGLGENBUFFERSPROC)loadGLProc("glGenBuffers");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)loadGLProc("glDeleteBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)loadGLProc("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)loadGLProc("glBufferData");
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC)loadGLProc("glBufferSubData");

    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)loadGLProc("glGenVertexArrays");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)loadGLProc("glDeleteVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)loadGLProc("glBindVertexArray");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)loadGLProc("glEnableVertexAttribArray");
    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)loadGLProc("glDisableVertexAttribArray");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)loadGLProc("glVertexAttribPointer");

    glCreateShader = (PFNGLCREATESHADERPROC)loadGLProc("glCreateShader");
    glDeleteShader = (PFNGLDELETESHADERPROC)loadGLProc("glDeleteShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)loadGLProc("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)loadGLProc("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)loadGLProc("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)loadGLProc("glGetShaderInfoLog");

    glCreateProgram = (PFNGLCREATEPROGRAMPROC)loadGLProc("glCreateProgram");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)loadGLProc("glDeleteProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)loadGLProc("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)loadGLProc("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)loadGLProc("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)loadGLProc("glGetProgramInfoLog");
    glUseProgram = (PFNGLUSEPROGRAMPROC)loadGLProc("glUseProgram");

    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)loadGLProc("glGetUniformLocation");
    glUniform1i = (PFNGLUNIFORM1IPROC)loadGLProc("glUniform1i");
    glUniform1f = (PFNGLUNIFORM1FPROC)loadGLProc("glUniform1f");
    glUniform2f = (PFNGLUNIFORM2FPROC)loadGLProc("glUniform2f");
    glUniform3f = (PFNGLUNIFORM3FPROC)loadGLProc("glUniform3f");
    glUniform4f = (PFNGLUNIFORM4FPROC)loadGLProc("glUniform4f");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)loadGLProc("glUniformMatrix4fv");

    glTexImage3D = (PFNGLTEXIMAGE3DPROC)loadGLProc("glTexImage3D");
    glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)loadGLProc("glTexSubImage3D");
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)loadGLProc("glActiveTexture");
    glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)loadGLProc("glGenerateMipmap");

    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)loadGLProc("glGenFramebuffers");
    glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)loadGLProc("glDeleteFramebuffers");
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)loadGLProc("glBindFramebuffer");
    glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)loadGLProc("glFramebufferTexture2D");
    glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)loadGLProc("glCheckFramebufferStatus");
    glDrawBuffers = (PFNGLDRAWBUFFERSPROC)loadGLProc("glDrawBuffers");

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)loadGLProc("wglSwapIntervalEXT");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)loadGLProc("wglCreateContextAttribsARB");

    if (!glGenBuffers || !glBindBuffer || !glBufferData || !glGenVertexArrays || !glBindVertexArray ||
        !glCreateShader || !glCompileShader || !glCreateProgram || !glLinkProgram) {
        std::cerr << "Failed to load critical OpenGL function pointers!" << std::endl;
        return false;
    }

    std::cout << "Modern OpenGL function pointers loaded successfully!" << std::endl;
    return true;
}

} // namespace Aetheria
