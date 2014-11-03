/*
Copyright © 2014 Paul Wortmann

This file is part of FLARE.

FLARE is free software: you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

FLARE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
FLARE.  If not, see http://www.gnu.org/licenses/
*/

#include "OpenGL_EXT.h"

namespace glExt
{
    PFNGLACTIVETEXTUREARBPROC         glActiveTextureARB         = NULL;
    PFNGLMULTITEXCOORD2FARBPROC       glMultiTexCoord2fARB       = NULL;
    PFNGLGENVERTEXARRAYSPROC          glGenVertexArrays          = NULL;
    PFNGLBINDVERTEXARRAYPROC          glBindVertexArray          = NULL;
    PFNGLGENBUFFERSPROC               glGenBuffers               = NULL;
    PFNGLBINDBUFFERPROC               glBindBuffer               = NULL;
    PFNGLBUFFERDATAPROC               glBufferData               = NULL;
    PFNGLVERTEXATTRIBPOINTERPROC      glVertexAttribPointer      = NULL;
    PFNGLENABLEVERTEXATTRIBARRAYPROC  glEnableVertexAttribArray  = NULL;
    PFNGLCREATESHADERPROC             glCreateShader             = NULL;
    PFNGLSHADERSOURCEPROC             glShaderSource             = NULL;
    PFNGLCOMPILESHADERPROC            glCompileShader            = NULL;
    PFNGLGETSHADERIVPROC              glGetShaderiv              = NULL;
    PFNGLGETSHADERINFOLOGPROC         glGetShaderInfoLog         = NULL;
    PFNGLCREATEPROGRAMPROC            glCreateProgram            = NULL;
    PFNGLATTACHSHADERPROC             glAttachShader             = NULL;
    PFNGLBINDATTRIBLOCATIONPROC       glBindAttribLocation       = NULL;
    PFNGLLINKPROGRAMPROC              glLinkProgram              = NULL;
    PFNGLGETPROGRAMIVPROC             glGetProgramiv             = NULL;
    PFNGLGETPROGRAMINFOLOGPROC        glGetProgramInfoLog        = NULL;
    PFNGLUSEPROGRAMPROC               glUseProgram               = NULL;
    PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = NULL;
    PFNGLDETACHSHADERPROC             glDetachShader             = NULL;
    PFNGLDELETEPROGRAMPROC            glDeleteProgram            = NULL;
    PFNGLDELETESHADERPROC             glDeleteShader             = NULL;
    PFNGLDELETEBUFFERSPROC            glDeleteBuffers            = NULL;
    PFNGLDELETEVERTEXARRAYSPROC       glDeleteVertexArrays       = NULL;
    PFNGLGETSTRINGIPROC               glGetStringi               = NULL;
    PFNGLGETATTRIBLOCATIONPROC        glGetAttribLocation        = NULL;
    PFNGLGETUNIFORMLOCATIONPROC       glGetUniformLocation       = NULL;
    PFNGLUNIFORM1IPROC                glUniform1i                = NULL;
    PFNGLUNIFORM4FVPROC               glUniform4fv               = NULL;
    void init(void **context)
    {
        if (context != NULL)
        {
            glActiveTextureARB         = (PFNGLACTIVETEXTUREARBPROC)         glXGetProcAddressARB((const unsigned char*)"glActiveTextureARB");
            glMultiTexCoord2fARB       = (PFNGLMULTITEXCOORD2FARBPROC)       glXGetProcAddressARB((const unsigned char*)"glMultiTexCoord2fARB");
            glGenVertexArrays          = (PFNGLGENVERTEXARRAYSPROC)          glXGetProcAddressARB((const unsigned char*)"glGenVertexArrays");
            glBindVertexArray          = (PFNGLBINDVERTEXARRAYPROC)          glXGetProcAddressARB((const unsigned char*)"glBindVertexArray");
            glGenBuffers               = (PFNGLGENBUFFERSPROC)               glXGetProcAddressARB((const unsigned char*)"glGenBuffers");
            glBindBuffer               = (PFNGLBINDBUFFERPROC)               glXGetProcAddressARB((const unsigned char*)"glBindBuffer");
            glBufferData               = (PFNGLBUFFERDATAPROC)               glXGetProcAddressARB((const unsigned char*)"glBufferData");
            glVertexAttribPointer      = (PFNGLVERTEXATTRIBPOINTERPROC)      glXGetProcAddressARB((const unsigned char*)"glVertexAttribPointer");
            glEnableVertexAttribArray  = (PFNGLENABLEVERTEXATTRIBARRAYPROC)  glXGetProcAddressARB((const unsigned char*)"glEnableVertexAttribArray");
            glCreateShader             = (PFNGLCREATESHADERPROC)             glXGetProcAddressARB((const unsigned char*)"glCreateShader");
            glShaderSource             = (PFNGLSHADERSOURCEPROC)             glXGetProcAddressARB((const unsigned char*)"glShaderSource");
            glCompileShader            = (PFNGLCOMPILESHADERPROC)            glXGetProcAddressARB((const unsigned char*)"glCompileShader");
            glGetShaderiv              = (PFNGLGETSHADERIVPROC)              glXGetProcAddressARB((const unsigned char*)"glGetShaderiv");
            glGetShaderInfoLog         = (PFNGLGETSHADERINFOLOGPROC)         glXGetProcAddressARB((const unsigned char*)"glGetShaderInfoLog");
            glCreateProgram            = (PFNGLCREATEPROGRAMPROC)            glXGetProcAddressARB((const unsigned char*)"glCreateProgram");
            glAttachShader             = (PFNGLATTACHSHADERPROC)             glXGetProcAddressARB((const unsigned char*)"glAttachShader");
            glBindAttribLocation       = (PFNGLBINDATTRIBLOCATIONPROC)       glXGetProcAddressARB((const unsigned char*)"glBindAttribLocation");
            glLinkProgram              = (PFNGLLINKPROGRAMPROC)              glXGetProcAddressARB((const unsigned char*)"glLinkProgram");
            glGetProgramiv             = (PFNGLGETPROGRAMIVPROC)             glXGetProcAddressARB((const unsigned char*)"glGetProgramiv");
            glGetProgramInfoLog        = (PFNGLGETPROGRAMINFOLOGPROC)        glXGetProcAddressARB((const unsigned char*)"glGetProgramInfoLog");
            glUseProgram               = (PFNGLUSEPROGRAMPROC)               glXGetProcAddressARB((const unsigned char*)"glUseProgram");
            glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) glXGetProcAddressARB((const unsigned char*)"glDisableVertexAttribArray");
            glDetachShader             = (PFNGLDETACHSHADERPROC)             glXGetProcAddressARB((const unsigned char*)"glDetachShader");
            glDeleteProgram            = (PFNGLDELETEPROGRAMPROC)            glXGetProcAddressARB((const unsigned char*)"glDeleteProgram");
            glDeleteShader             = (PFNGLDELETESHADERPROC)             glXGetProcAddressARB((const unsigned char*)"glDeleteShader");
            glDeleteBuffers            = (PFNGLDELETEBUFFERSPROC)            glXGetProcAddressARB((const unsigned char*)"glDeleteBuffers");
            glDeleteVertexArrays       = (PFNGLDELETEVERTEXARRAYSPROC)       glXGetProcAddressARB((const unsigned char*)"glDeleteVertexArrays");
            glGetStringi               = (PFNGLGETSTRINGIPROC)               glXGetProcAddressARB((const unsigned char*)"glGetStringi");
            glGetAttribLocation        = (PFNGLGETATTRIBLOCATIONPROC)        glXGetProcAddressARB((const unsigned char*)"glGetAttribLocation");
            glGetUniformLocation       = (PFNGLGETUNIFORMLOCATIONPROC)       glXGetProcAddressARB((const unsigned char*)"glGetUniformLocation");
            glUniform1i                = (PFNGLUNIFORM1IPROC)                glXGetProcAddressARB((const unsigned char*)"glUniform1i");
            glUniform4fv               = (PFNGLUNIFORM4FVPROC)               glXGetProcAddressARB((const unsigned char*)"glUniform4fv");
        }
    }
}
