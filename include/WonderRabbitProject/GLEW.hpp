#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <iostream>

#ifndef WRP_NOT_DEFINE_L
  #ifndef L
    #define L(a,b) 
  #endif
#endif

#define WRP_GL
namespace WonderRabbitProject { namespace GL {
  using GLboolean = bool;
  using GLbyte    = int8_t;
  using GLubyte   = uint8_t;
  using GLchar    = char;
  using GLshort   = int16_t;
  using GLushort  = uint16_t;
  using GLint     = int32_t;
  using GLuint    = uint32_t;
  //using GLfixed   = ?;
  using GLint64   = int64_t;
  using GLuint64  = uint64_t;
  using GLsizei   = uint32_t;
  using GLenum    = int32_t;
  //using GLintptr  = ?;
  using GLsizeiptr= size_t;
  using GLsync      = void*;
  using GLbitfield= uint32_t;
  //using GLhalf    = ?;
  using GLfloat   = float;
  using GLclampf  = float;
  using GLdouble  = double;
  using GLclampd  = double;
} }

#define WRP_GLEW
namespace WonderRabbitProject { namespace GLEW {
  namespace C
  {
    #include <GL/glew.h>
  }

  #include "./GLEW/undef_OpenGL_types.hpp"
  #include "./GLEW/SHADER.hpp"
  
  struct glew;

  template<GL::GLuint SHADER_TYPE>
  struct shader
  {
    static constexpr SHADER shader_type = SHADER(SHADER_TYPE);
    //shader() : data_(glew::instance().create_shader(shader_type))
    shader(GL::GLuint data) : data_(data) { }
  private:
    GL::GLuint data_;
  };

  using vertex_shader          = shader<GL::GLuint(SHADER::VERTEX)>;
  using fragment_shader        = shader<GL::GLuint(SHADER::FRAGMENT)>;
  using geometry_shader        = shader<GL::GLuint(SHADER::GEOMETRY)>;
  using tess_control_shader    = shader<GL::GLuint(SHADER::TESS_CONTROL)>;
  using tess_evaluation_shader = shader<GL::GLuint(SHADER::TESS_EVALUATION)>;

  struct glew final
  {
    using this_type = glew;
    
    glew(const this_type&) = delete;
    glew(this_type&&)      = delete;
    this_type& operator=(const this_type&) = delete;
    this_type& operator=(this_type&&)      = delete;
    
    template<GL::GLuint SHADER_TYPE>
    shader<SHADER_TYPE> create_shader()
    {
      L(INFO
      ,  "--> WRP::GLFW::glew::create_shader : "
      << to_string(SHADER(SHADER_TYPE))
      );
      return shader<SHADER_TYPE>(C::glCreateShader(GL::GLuint(SHADER_TYPE)));
    }
    // clean if C++ support enum class in template params direct.
    #define create_shader(a) create_shader<GL::GLuint(a)>()

    static this_type& instance()
    {
      L(INFO, "--> WRP::GLFW::glew::instance");
      if ( ! i )
      {
        std::lock_guard<decltype(m)> g(m);
        L(INFO, "lock_guard with mutex address is " << std::hex << &m);
        if ( ! i ) {
          i.reset( new this_type() );
          L(INFO, "new instance; address of instance is " << std::hex << i.get());
        }
      }
      L(INFO, "returning instance address is " << std::hex << i.get());
      L(INFO, "<-- WRP::GLFW::glew::instance");
      return *i;
    }
    
  private:
    static std::unique_ptr<this_type> i;
    static std::mutex m;

    glew()
    {
      L(INFO, "--> WRP::GLFW::glew::ctor");
      initialize();
      L(INFO, "<-- WRP::GLFW::glew::ctor");
    }

    void initialize()
    {
      L(INFO, "--> WRP::GLFW::glew::initialize");
      auto r = C::glewInit();
      if (r != GLEW_OK)
      {
        auto message = std::string("flewInit fail. code = ") + std::to_string(r);
        L(FATAL, message);
        throw std::runtime_error(message);
      }
      L(INFO, "glewInit done");
      L(INFO, "<-- WRP::GLFW::glew::initialize");
    }

  };
  
  std::mutex glew::m;
  std::unique_ptr<glew> glew::i;

} }

