#pragma once

#include <cstddef>
#include <cstdint>
#include <stack>
#include <array>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <iostream>
#include <utility>

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
  #include "./GLEW/enum/SHADER.hpp"
  #include "./GLEW/enum/ERROR.hpp"
  #include "./GLEW/enum/VERTEX_ATTRIBUTE.hpp"
  #include "./GLEW/enum/USAGE.hpp"
  #include "./GLEW/enum/MODE.hpp"

  #include "./GLEW/basic_types.hpp"
  #include "./GLEW/destruct_invoker.hpp"

  #include "./GLEW/shader.hpp"
  #include "./GLEW/program.hpp"
  #include "./GLEW/vertex.hpp"
  #include "./GLEW/model.hpp"

  struct glew final
  {
    using this_type = glew;
    
    glew(const this_type&) = delete;
    glew(this_type&&)      = delete;
    this_type& operator=(const this_type&) = delete;
    this_type& operator=(this_type&&)      = delete;
    
    template<class TSHADER>
    inline TSHADER create_shader() const
    {
      L(INFO
      ,  "--> WRP::GLEW::glew::create_shader : "
      << to_string(TSHADER::shader_type)
      );
      return TSHADER();
    }
    
    inline program create_program() const
    {
      L(INFO,  "--> WRP::GLEW::glew::create_program");
      return program();
    }

    template
    <
      class TVERTEX,
      class TUSAGE,
      class TDEFAULT_INVOKE_MODE
    >
    inline model_vi
    <
      TVERTEX, TUSAGE, TDEFAULT_INVOKE_MODE
    >
    create_model(
      std::vector<TVERTEX>&& data_vertices,
      std::vector<GL::GLuint>&& data_indices
    ) const
    {
      L(INFO,  "--> WRP::GLEW::glew::create_model(vertices,indices)");
      return model_vi
        <TVERTEX, TUSAGE, TDEFAULT_INVOKE_MODE>
        ( std::move(data_vertices), std::move(data_indices) );
    }
    
    template < class TVERTEX, class TUSAGE>
    inline model_vi< TVERTEX, TUSAGE, mode_points >
    create_model(
      std::vector<TVERTEX>&& data_vertices,
      std::vector<GL::GLuint>&& data_indices
    ) const
    {
      return create_model
        <TVERTEX, TUSAGE, mode_points>
        (std::move(data_vertices), std::move(data_indices));
    }

    template <class TVERTEX>
    inline model_vi< TVERTEX, usage_static_draw, mode_points >
    create_model(
      std::vector<TVERTEX>&& data_vertices,
      std::vector<GL::GLuint>&& data_indices
    ) const
    {
      return create_model
        <TVERTEX, usage_static_draw>
        (std::move(data_vertices), std::move(data_indices));
    }
    
    template
    <
      class TVERTEX,
      class TUSAGE,
      class TDEFAULT_INVOKE_MODE
    >
    inline model_v
    <
      TVERTEX, TUSAGE, TDEFAULT_INVOKE_MODE
    >
    create_model(
      std::vector<TVERTEX>&& data
    ) const
    {
      L(INFO,  "--> WRP::GLEW::glew::create_model");
      return model_v
        <TVERTEX, TUSAGE, TDEFAULT_INVOKE_MODE>
        (std::move(data));
    }

    /*
    template<class TS ...>
    inline texture<> texture(std::tuple<,>, TS ... ts)
    {
      return texture_();
    }
    */

    inline destruct_invoker
    enable_vertex_attribute(GL::GLuint v)
    {
      std::vector<GL::GLuint> list;
      list.emplace_back(v);
      return enable_vertex_attributes(std::move(list));
    }

    inline destruct_invoker
    enable_vertex_attributes(std::vector<GL::GLuint>&& list)
    {
      for(const auto& n : list)
        C::glEnableVertexAttribArray(n);
      return
      {
        [list]()
        {
          std::for_each(
            list.crbegin(), list.crend(),
            [](GL::GLuint n){ C::glDisableVertexAttribArray(n); }
          );
        }
      };
    }

    template < class TVERTEX, class TUSAGE>
    inline model_v< TVERTEX, TUSAGE, mode_points >
    create_model( std::vector<TVERTEX>&& data) const
    {
      return create_model
        <TVERTEX, TUSAGE, mode_points>(std::move(data));
    }

    template < class TVERTEX>
    inline model_v< TVERTEX, usage_static_draw, mode_points >
    create_model( std::vector<TVERTEX>&& data) const
    {
      return create_model
        <TVERTEX, usage_static_draw>(std::move(data));
    }

    inline void use_program(const program& p) const
    {
      //L(INFO,
      //  "--> WRP::GLEW::glew::use_program; program"
      //  "[" << p.program_ << "] (program object address is " << &p << ")"
      //);
      C::glUseProgram(p.program_);
      //WRP_GLEW_TEST_ERROR
    }

    // float
    inline void uniform(
      GL::GLint location,
      const GL::GLfloat value
    ) const
    { C::glUniform1f(location, value); }
    
    template<size_t N>
    inline void uniform(
      GL::GLint location,
      const vec<vec<GL::GLfloat, 1>, N>& values
    ) const
    { C::glUniform1fv(location, N, values.data()->data()); }
    
    inline void uniform(
      GL::GLint location,
      const u_vec2<GL::GLfloat>& values
    ) const
    { C::glUniform2fv(location, 1, values.data()); }
    
    template<size_t N>
    inline void uniform(
      GL::GLint location,
      const vec<u_vec2<GL::GLfloat>, N>& values
    ) const
    { C::glUniform2fv(location, N, values.data()->data()); }
    
    inline void uniform(
      GL::GLint location,
      const u_vec3<GL::GLfloat>& values
    ) const
    { C::glUniform3fv(location, 1, values.data()); }
    
    template<size_t N>
    inline void uniform(
      GL::GLint location,
      const vec<u_vec3<GL::GLfloat>, N>& values
    ) const
    { C::glUniform3fv(location, N, values.data()->data()); }
    
    inline void uniform(
      GL::GLint location,
      const u_vec4<GL::GLfloat>& values
    ) const
    { C::glUniform4fv(location, 1, values.data()); }
    
    template<size_t N>
    inline void uniform(
      GL::GLint location,
      const vec<u_vec4<GL::GLfloat>, N>& values
    ) const
    { C::glUniform4fv(location, N, values.data()->data()); }
    
    // int
    inline void uniform(
      GL::GLint location,
      const GL::GLint value
    ) const
    { C::glUniform1i(location, value); }
    
    template<size_t N>
    inline void uniform(
      GL::GLint location,
      const vec<vec<GL::GLint, 1>, N>& values
    ) const
    { C::glUniform1iv(location, N, values.data()->data()); }
    
    inline void uniform(
      GL::GLint location,
      const u_vec2<GL::GLint>& values
    ) const
    { C::glUniform2iv(location, 1, values.data()); }
    
    template<size_t N>
    inline void uniform(
      GL::GLint location,
      const vec<u_vec2<GL::GLint>, N>& values
    ) const
    { C::glUniform2iv(location, N, values.data()->data()); }
    
    inline void uniform(
      GL::GLint location,
      const u_vec3<GL::GLint>& values
    ) const
    { C::glUniform3iv(location, 1, values.data()); }
    
    template<size_t N>
    inline void uniform(
      GL::GLint location,
      const vec<u_vec3<GL::GLint>, N>& values
    ) const
    { C::glUniform3iv(location, N, values.data()->data()); }
    
    inline void uniform(
      GL::GLint location,
      const u_vec4<GL::GLint>& values
    ) const
    { C::glUniform4iv(location, 1, values.data()); }
    
    template<size_t N>
    inline void uniform(
      GL::GLint location,
      const vec<u_vec4<GL::GLint>, N>& values
    ) const
    { C::glUniform4iv(location, N, values.data()->data()); }
    
    // uint
    inline void uniform(
      GL::GLint location,
      const GL::GLuint value
    ) const
    { C::glUniform1i(location, value); }
    
    template<size_t N>
    inline void uniform(
      GL::GLint location,
      const vec<vec<GL::GLuint, 1>, N>& values
    ) const
    { C::glUniform1uiv(location, N, values.data()->data()); }
    
    inline void uniform(
      GL::GLint location,
      const u_vec2<GL::GLuint>& values
    ) const
    { C::glUniform2uiv(location, 1, values.data()); }
    
    template<size_t N>
    inline void uniform(
      GL::GLint location,
      const vec<u_vec2<GL::GLuint>, N>& values
    ) const
    { C::glUniform2uiv(location, N, values.data()->data()); }
    
    inline void uniform(
      GL::GLint location,
      const u_vec3<GL::GLuint>& values
    ) const
    { C::glUniform3uiv(location, 1, values.data()); }
    
    template<size_t N>
    inline void uniform(
      GL::GLint location,
      const vec<u_vec3<GL::GLuint>, N>& values
    ) const
    { C::glUniform3uiv(location, N, values.data()->data()); }
    
    inline void uniform(
      GL::GLint location,
      const u_vec4<GL::GLuint>& values
    ) const
    { C::glUniform4uiv(location, 1, values.data()); }
    
    template<class TMODE, class TMODEL>
    inline void invoke(const TMODEL& m) const
    {
      m.invoke<TMODE>();
    }

    inline void invoke(const model& m) const
    { 
      m.invoke();
    }

    inline void flush() const
    { C::glFlush(); }

    inline destruct_invoker flusher() const
    { return { [this]{ this->flush(); } }; }

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

