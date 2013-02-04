#pragma once

#include <functional>
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
  struct program;

  template<GL::GLuint SHADER_TYPE>
  struct shader
  {
    friend glew;
    friend program;
    static constexpr SHADER shader_type = SHADER(SHADER_TYPE);
    using this_type = shader<SHADER_TYPE>;
    
    shader(this_type&& o)
      : shader_(std::move(o.shader_))
    { o.cancel(); }
    shader(const this_type&) = delete;
    
    this_type& operator=(this_type&&)      = delete;
    this_type& operator=(const this_type&) = delete;

    inline void cancel()
    {
      L(INFO, "WRP::GLEW::shader<"
        << to_string(shader_type)
        << ">::calcel; this address is "
        << std::hex << this
      );
      finalizer_ = []{};
    }
    
    inline void source(std::string&& v)
    {
      L(INFO, "WRP::GLEW::shader<"
        << to_string(shader_type)
        << ">::source(std::string&&)"
      );
      source_ = std::move(v);
    }

    inline void source(std::istream&& s)
    {
      L(INFO, "WRP::GLEW::shader<"
        << to_string(shader_type)
        << ">::source(std::istream)"
      );
      s.seekg(0, std::ios::end);
      auto size = s.tellg();
      s.seekg(0, std::ios::beg);
      source_.resize(size);
      s.read(const_cast<char*>(source_.data()), size);
    }

    inline const std::string& source() const
    { return source_; }

    inline void compile() const
    {
      L(INFO, "WRP::GLEW::shader<" << to_string(shader_type) << ">::compile");
      {
        constexpr C::GLsizei  count = 1;
        const     C::GLchar*  str   = source_.data();
        const     C::GLint    size  = source_.size();
        C::glShaderSource(shader_, count, &str, &size);
        L(INFO, "glShaderSource done");
      }
      C::glCompileShader(shader_);
      {
        C::GLint result;
        C::glGetShaderiv(shader_, GL_COMPILE_STATUS, &result);
        if( ! result )
        {
          L(ERROR, "glCompileShader fail");
          std::string error_log;
          C::GLint size;
          C::glGetShaderiv(shader_, GL_INFO_LOG_LENGTH, &size);
          error_log.resize(size);
          C::GLint size_written;
          C::glGetShaderInfoLog(
            shader_, size, &size_written,
            const_cast<char*>(error_log.data())
          );
          L(INFO, "error log written-size / size : " << size_written << " / " << size);
          L(ERROR,"error log : " <<  error_log);
          throw std::runtime_error(error_log);
        }
      }
      L(INFO, "glCompileShader done");
    }

    ~shader()
    {
      L(INFO, "WRP::GLEW::shader<" << to_string(shader_type) << ">::dtor");
      finalizer_();
    }
  private:
    shader()
      : shader_(C::glCreateShader(GL::GLuint(shader_type)))
      , finalizer_([this]{
        L(INFO, "shader object = " << std::hex << this->shader_);
        C::glDeleteShader(this->shader_);
        L(INFO, "glDeleteShader done");
      })
    {
      L(INFO, "WRP::GLEW::shader<" << to_string(shader_type) << ">::ctor");
      L(INFO, "shader object = " << std::hex << shader_);
    }
    
    GL::GLuint shader_;
    std::function<void()> finalizer_;
    std::string source_;
  };

  using vertex_shader          = shader<GL::GLuint(SHADER::VERTEX)>;
  using fragment_shader        = shader<GL::GLuint(SHADER::FRAGMENT)>;
  using geometry_shader        = shader<GL::GLuint(SHADER::GEOMETRY)>;
  using tess_control_shader    = shader<GL::GLuint(SHADER::TESS_CONTROL)>;
  using tess_evaluation_shader = shader<GL::GLuint(SHADER::TESS_EVALUATION)>;

  struct program
  {
    friend glew;
    
    using this_type = program;

    program(const this_type&) = delete;
    program(this_type&& o)
      : program_(std::move(o.program_))
      , finalizer_(std::move(o.finalizer_))
    { }

    this_type& operator=(const this_type&) = delete;
    this_type& operator=(this_type&&)      = delete;

    inline void cancel()
    { finalizer_ = []{}; }
    
    template<class T>
    inline void attach(const T& shader_) const
    {
      L(INFO, "WRP::GLEW::program::attach; shader address is "
        << std::hex << &shader_
      );
      C::glAttachShader(program_, shader_.shader_);
    }
    
    template<class T, class ... TS>
    inline void attach(const T& shader_, const TS& ... ts) const
    {
      attach(shader_);
      attach(ts ...);
    }

    inline void bind_vs(const GL::GLuint index, const std::string& name)
      const
    {
      L(INFO,
        "WRP::GLEW::program::bind_vs; "
        "[" << index << "]:" << name
      );
      C::glBindAttribLocation(program_, index, name.data());
    }
    
    template<class ... TS>
    inline void bind_vs(const std::string& name, const TS& ... ts)
      const
    {
      bind_vs_(0, name, ts ...);
    }
    
    inline void bind_fs(const GL::GLuint index, const std::string& name)
      const
    {
      L(INFO,
        "WRP::GLEW::program::bind_fs; "
        "[" << index << "]:" << name
      );
      C::glBindFragDataLocation(program_, index, name.data());
    }
    
    template<class ... TS>
    inline void bind_fs(const std::string& name, const TS& ... ts)
      const
    {
      bind_fs_(0, name, ts ...);
    }
    
    inline void link() const
    {
      L(INFO, "WRP::GLEW::program::link");
      
      C::glLinkProgram(program_);
      C::GLint result;
      C::glGetProgramiv(program_, GL_LINK_STATUS, &result);
      if( ! result )
      {
        auto message = "fail";
        L(ERROR, message);
        throw std::runtime_error(message);
      }
      L(INFO, "succeed");
    }
    
  private:
    program()
      : finalizer_([this]{C::glDeleteProgram(this->program_);})
    {
      L(INFO, "WRP::GLEW::program::ctor");
      L(INFO, "program object = " << std::hex << program_);
    }
    
    inline void attach() const { }
    
    inline void bind_vs_(size_t) const {}
    
    template<class ... TS>
    inline void bind_vs_(size_t index, const std::string& name, const TS& ... ts)
      const
    { 
      bind_vs(index, name);
      bind_vs_(index + 1, ts ...);
    }
    
    inline void bind_fs_(size_t) const {}
    
    template<class ... TS>
    inline void bind_fs_(size_t index, const std::string& name, const TS& ... ts)
      const
    { 
      bind_fs(index, name);
      bind_fs_(index + 1, ts ...);
    }
    
    GL::GLuint program_;
    std::function<void()> finalizer_;
    
  };

  struct glew final
  {
    using this_type = glew;
    
    glew(const this_type&) = delete;
    glew(this_type&&)      = delete;
    this_type& operator=(const this_type&) = delete;
    this_type& operator=(this_type&&)      = delete;
    
    template<class TSHADER>
    TSHADER create_shader() const
    {
      L(INFO
      ,  "--> WRP::GLEW::glew::create_shader : "
      << to_string(TSHADER::shader_type)
      );
      return TSHADER();
    }
    
    program create_program() const
    {
      L(INFO,  "--> WRP::GLEW::glew::create_program");
      return program();
    }

    void use_program(const program& p) const
    {
      L(INFO, "--> WRP::GLEW::glew::use_program; program address is " << &p);
      C::glUseProgram(p.program_);
    }

    static this_type& instance()
    {
      L(INFO, "--> WRP::GLEW::glew::instance");
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
      L(INFO, "<-- WRP::GLEW::glew::instance");
      return *i;
    }
    
  private:
    static std::unique_ptr<this_type> i;
    static std::mutex m;

    glew()
    {
      L(INFO, "--> WRP::GLEW::glew::ctor");
      initialize();
      L(INFO, "<-- WRP::GLEW::glew::ctor");
    }

    void initialize()
    {
      L(INFO, "--> WRP::GLEW::glew::initialize");
      auto r = C::glewInit();
      if (r != GLEW_OK)
      {
        auto message = std::string("flewInit fail. code = ") + std::to_string(r);
        L(FATAL, message);
        throw std::runtime_error(message);
      }
      L(INFO, "glewInit done");
      L(INFO, "<-- WRP::GLEW::glew::initialize");
    }

  };
  
  std::mutex glew::m;
  std::unique_ptr<glew> glew::i;

} }

