#pragma once

#include "./detail/program_uniform_delegate.hpp"

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
    L(INFO, "WRP::GLEW::program::attach; shader"
      << "[" << shader_.shader_ << "] address is "
      << std::hex << &shader_
    );
    C::glAttachShader(program_, shader_.shader_);
    WRP_GLEW_TEST_ERROR
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
    {
      C::GLint result = false;
      C::glGetProgramiv(program_, GL_LINK_STATUS, &result);
      if( ! result )
      {
        L(ERROR, "glLinkProgram fail");
        std::string error_log;
        C::GLint size = 0;
        C::glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &size);
        error_log.resize(size);
        C::GLint size_written = 0;
        C::glGetProgramInfoLog(
          program_, size, &size_written,
          const_cast<char*>(error_log.data())
        );
        L(INFO, "error log written-size / size : " << size_written << " / " << size);
        L(ERROR,"error log : " <<  error_log);
        throw std::runtime_error(error_log);
      }
      L(INFO, "succeed");
    }
  }

  inline GL::GLint active_attributes() const
  {
    GL::GLint r;
    GLEW::C::glGetProgramiv(program_, GL_ACTIVE_ATTRIBUTES, &r);
    return r;
  }
  
  inline GL::GLuint where_bind_vs(const std::string& s) const
  {
    auto r = C::glGetAttribLocation(program_, s.data());
    if( r == -1 )
    {
      auto message = std::string("glGetAttribLocation(")
        + std::to_string(program_) + "," + s + ") return " + std::to_string(r);
      L(ERROR, message);
      throw std::runtime_error(message);
    }
    return r;
  }

  inline GL::GLuint where_bind_fs(const std::string& s) const
  {
    auto r = C::glGetFragDataLocation(program_, s.data());
    if( r == -1 )
    {
      auto message = std::string("glGetFragDataLocation(")
        + std::to_string(program_) + "," + s + ") return " + std::to_string(r);
      L(ERROR, message);
      throw std::runtime_error(message);
    }
    return r;
  }

  inline GL::GLuint where_uniform(std::string&& name) const
  { return C::glGetUniformLocation(program_, name.data()); }

  template<class T, size_t N = 1>
  decltype(detail::program_uniform_delegate<T, N>()(GL::GLuint(0), GL::GLint(0)))
  uniform(GL::GLint location)
  { return detail::program_uniform_delegate<T, N>()(program_, location); }

private:
  program()
    : program_(C::glCreateProgram())
    , finalizer_([this]{C::glDeleteProgram(this->program_);})
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

