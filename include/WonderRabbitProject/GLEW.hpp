#pragma once

#include <stack>
#include <array>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <iostream>

#include <boost/type_traits.hpp>
#include <boost/mpl/map.hpp>
//#include <boost/mpl/filter_view.hpp>
#include <boost/mpl/find_if.hpp>

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
  #include "./GLEW/ERROR.hpp"
  #include "./GLEW/VERTEX_ATTRIBUTE.hpp"
  #include "./GLEW/USAGE.hpp"
  #include "./GLEW/MODE.hpp"

  #include "./GLEW/basic_types.hpp"
  #include "./GLEW/destruct_invoker.hpp"

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
        C::GLint result = false;
        C::glGetShaderiv(shader_, GL_COMPILE_STATUS, &result);
        if( ! result )
        {
          L(ERROR, "glCompileShader fail");
          std::string error_log;
          C::GLint size = 0;
          C::glGetShaderiv(shader_, GL_INFO_LOG_LENGTH, &size);
          error_log.resize(size);
          C::GLint size_written = 0;
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
    
    inline GL::GLuint where_bind_vs(std::string&& s) const
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

    inline GL::GLuint where_bind_fs(std::string&& s) const
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

  struct model
  {
    friend glew;
    virtual ~model() { }
  protected:
    virtual void invoke() const = 0;
  };

  template
  <
    class TELEMENT,
    size_t TELEMENT_SIZE,
    class TUSAGE,
    class TDEFAULT_INVOKE_MODE
  >
  struct model_v final : model
  {
    friend glew;
    using element_type = TELEMENT;
    static constexpr size_t element_size = TELEMENT_SIZE;
    using data_type = std::vector<std::array<element_type ,element_size>>;
    static constexpr USAGE usage = USAGE(TUSAGE::value);
    static constexpr MODE default_invoke_mode = TDEFAULT_INVOKE_MODE::value;
  private:
    using va_candidates = boost::mpl::map
    < boost::mpl::pair< GL::GLfloat , boost::mpl::int_<GL::GLenum(VERTEX_ATTRIBUTE::BINARY32)> >
    , boost::mpl::pair< GL::GLdouble, boost::mpl::int_<GL::GLenum(VERTEX_ATTRIBUTE::BINARY64)> > 
    , boost::mpl::pair< GL::GLbyte  , boost::mpl::int_<GL::GLenum(VERTEX_ATTRIBUTE::INT8    )> > 
    , boost::mpl::pair< GL::GLubyte , boost::mpl::int_<GL::GLenum(VERTEX_ATTRIBUTE::UINT8   )> > 
    , boost::mpl::pair< GL::GLshort , boost::mpl::int_<GL::GLenum(VERTEX_ATTRIBUTE::INT16   )> > 
    , boost::mpl::pair< GL::GLushort, boost::mpl::int_<GL::GLenum(VERTEX_ATTRIBUTE::UINT16  )> > 
    , boost::mpl::pair< GL::GLint   , boost::mpl::int_<GL::GLenum(VERTEX_ATTRIBUTE::INT32   )> > 
    , boost::mpl::pair< GL::GLuint  , boost::mpl::int_<GL::GLenum(VERTEX_ATTRIBUTE::UINT32  )> > 
    >;

    using mpl_va_type = 
      typename boost::mpl::second<
        typename boost::mpl::deref<
          typename boost::mpl::find_if<
            va_candidates,
            boost::is_same< element_type, boost::mpl::first<boost::mpl::_> >
          >::type
        >::type
      >::type;
  public:
    static constexpr VERTEX_ATTRIBUTE vertex_attribute
      = VERTEX_ATTRIBUTE(mpl_va_type::value);
      
    ~model_v() override
    {
      C::glDeleteBuffers(buffer_count, &vertex_buffer);
      C::glDeleteVertexArrays(buffer_count, &vertex_arrays);
    }

  private:
    static constexpr size_t buffer_count = 1;
    GL::GLuint vertex_buffer;
    GL::GLuint vertex_arrays;
    GL::GLuint vertices;

    model_v(data_type&& data)
    {
      vertices = data.size();
      C::glGenVertexArrays(buffer_count, &vertex_arrays);
      C::glBindVertexArray(vertex_arrays);
      C::glGenBuffers(buffer_count, &vertex_buffer);
      C::glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
      C::glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(element_type) * element_size * vertices,
        data.data(),
        GL::GLenum(usage)
      );
      C::glBindBuffer(GL_ARRAY_BUFFER, 0);
      C::glBindVertexArray(0);
    }
    
    template<class TMODE>
    void invoke() const override
    {
      C::glBindVertexArray(vertex_arrays);
      C::glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
      C::glVertexAttribPointer(
        0, element_size, GL::GLenum(vertex_attribute), false, 0, 0
      );
      C::glDrawArrays(GL::GLenum(TMODE::value), 0, vertices);
      C::glBindBuffer(GL_ARRAY_BUFFER, 0);
      //C::glBindVertexArray(0);
    }
    
    void invoke() const override
    { invoke<TDEFAULT_INVOKE_MODE>(); }
  };

  template
  <
    class TELEMENT,
    size_t TELEMENT_SIZE,
    class TUSAGE,
    class TDEFAULT_INVOKE_MODE
  >
  struct model_vi final : model
  {
    friend glew;
    using element_type = TELEMENT;
    static constexpr size_t element_size = TELEMENT_SIZE;
    using data_vertices_type = std::vector<std::array<element_type ,element_size>>;
    static constexpr USAGE usage = USAGE(TUSAGE::value);
    static constexpr MODE default_invoke_mode = TDEFAULT_INVOKE_MODE::value;
    using data_indices_element_type = GL::GLuint;
    using data_indices_type
      = std::vector<data_indices_element_type>;
  private:
    using va_candidates = boost::mpl::map
    < boost::mpl::pair< GL::GLfloat , boost::mpl::int_<GL::GLenum(VERTEX_ATTRIBUTE::BINARY32)> >
    , boost::mpl::pair< GL::GLdouble, boost::mpl::int_<GL::GLenum(VERTEX_ATTRIBUTE::BINARY64)> > 
    , boost::mpl::pair< GL::GLbyte  , boost::mpl::int_<GL::GLenum(VERTEX_ATTRIBUTE::INT8    )> > 
    , boost::mpl::pair< GL::GLubyte , boost::mpl::int_<GL::GLenum(VERTEX_ATTRIBUTE::UINT8   )> > 
    , boost::mpl::pair< GL::GLshort , boost::mpl::int_<GL::GLenum(VERTEX_ATTRIBUTE::INT16   )> > 
    , boost::mpl::pair< GL::GLushort, boost::mpl::int_<GL::GLenum(VERTEX_ATTRIBUTE::UINT16  )> > 
    , boost::mpl::pair< GL::GLint   , boost::mpl::int_<GL::GLenum(VERTEX_ATTRIBUTE::INT32   )> > 
    , boost::mpl::pair< GL::GLuint  , boost::mpl::int_<GL::GLenum(VERTEX_ATTRIBUTE::UINT32  )> > 
    >;

    using mpl_va_type = 
      typename boost::mpl::second<
        typename boost::mpl::deref<
          typename boost::mpl::find_if<
            va_candidates,
            boost::is_same< element_type, boost::mpl::first<boost::mpl::_> >
          >::type
        >::type
      >::type;
  public:
    static constexpr VERTEX_ATTRIBUTE vertex_attribute
      = VERTEX_ATTRIBUTE(mpl_va_type::value);
      
    ~model_vi() override
    {
      C::glDeleteBuffers(buffer_count, buffer.data());
    }

  private:
    static constexpr size_t buffer_count = 2;
    static constexpr size_t vertex_buffer_id = 0;
    static constexpr size_t index_buffer_id  = vertex_buffer_id + 1;
    std::array<GL::GLuint, buffer_count> buffer;
    GL::GLuint
      num_of_vertices,
      num_of_indices;
    
    const GL::GLuint& vertex_buffer() const { return buffer[vertex_buffer_id]; }
    const GL::GLuint& index_buffer() const { return buffer[index_buffer_id]; }

    model_vi(data_vertices_type&& data_vertices, data_indices_type&& data_indices)
    {
      num_of_vertices = data_vertices.size();
      num_of_indices  = data_indices.size();
      C::glGenBuffers(buffer_count, buffer.data());
      C::glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer());
      C::glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(element_type) * element_size * num_of_vertices,
        data_vertices.data(),
        GL::GLenum(usage)
      );
      C::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer());
      L(INFO, "AAA");
      C::glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(data_indices_element_type) * num_of_indices,
        data_indices.data(),
        GL::GLenum(usage)
      ); 
      L(INFO, "BBB");
      C::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      C::glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    template<class TMODE>
    void invoke() const override
    {
      C::glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer());
      C::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer());
      C::glVertexAttribPointer(
        0, element_size, GL::GLenum(vertex_attribute), false, 0, 0
      );
      C::glDrawElements(
        GL::GLenum(TMODE::value),
        num_of_indices,
        GL::GLenum(VERTEX_ATTRIBUTE::UINT32),
        0
      );
      C::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      C::glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    void invoke() const override
    { invoke<TDEFAULT_INVOKE_MODE>(); }
  };
  
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
      class TELEMENT_TYPE,
      size_t TELEMENT_SIZE,
      class TUSAGE,
      class TDEFAULT_INVOKE_MODE
    >
    inline model_vi
    <
      TELEMENT_TYPE, TELEMENT_SIZE, TUSAGE, TDEFAULT_INVOKE_MODE
    >
    create_model(
      std::vector<std::array<TELEMENT_TYPE, TELEMENT_SIZE>>&& data_vertices,
      std::vector<GL::GLuint>&& data_indices
    ) const
    {
      L(INFO,  "--> WRP::GLEW::glew::create_model(vertices,indices)");
      return model_vi
        <TELEMENT_TYPE, TELEMENT_SIZE, TUSAGE, TDEFAULT_INVOKE_MODE>
        ( std::move(data_vertices), std::move(data_indices) );
    }
    
    template < class TELEMENT_TYPE, size_t TELEMENT_SIZE, class TUSAGE>
    inline model_vi< TELEMENT_TYPE, TELEMENT_SIZE, TUSAGE, mode_points >
    create_model(
      std::vector<std::array<TELEMENT_TYPE, TELEMENT_SIZE>>&& data_vertices,
      std::vector<std::array<GL::GLuint, 2>>&& data_indices
    ) const
    {
      return create_model
        <TELEMENT_TYPE, TELEMENT_SIZE, TUSAGE, mode_points>
        (std::move(data_vertices), std::move(data_indices));
    }

    template < class TELEMENT_TYPE, size_t TELEMENT_SIZE>
    inline model_vi< TELEMENT_TYPE, TELEMENT_SIZE, usage_static_draw, mode_points >
    create_model(
      std::vector<std::array<TELEMENT_TYPE, TELEMENT_SIZE>>&& data_vertices,
      std::vector<std::array<GL::GLuint, 2>>&& data_indices
    ) const
    {
      return create_model
        <TELEMENT_TYPE, TELEMENT_SIZE, usage_static_draw>
        (std::move(data_vertices), std::move(data_indices));
    }
    
    template
    <
      class TELEMENT_TYPE,
      size_t TELEMENT_SIZE,
      class TUSAGE,
      class TDEFAULT_INVOKE_MODE
    >
    inline model_v
    <
      TELEMENT_TYPE, TELEMENT_SIZE, TUSAGE, TDEFAULT_INVOKE_MODE
    >
    create_model(
      std::vector<std::array<TELEMENT_TYPE, TELEMENT_SIZE>>&& data
    ) const
    {
      L(INFO,  "--> WRP::GLEW::glew::create_model");
      return model_v
        <TELEMENT_TYPE, TELEMENT_SIZE, TUSAGE, TDEFAULT_INVOKE_MODE>
        (std::move(data));
    }

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

    template < class TELEMENT_TYPE, size_t TELEMENT_SIZE, class TUSAGE>
    inline model_v< TELEMENT_TYPE, TELEMENT_SIZE, TUSAGE, mode_points >
    create_model( std::vector<std::array<TELEMENT_TYPE, TELEMENT_SIZE>>&& data) const
    {
      return create_model
        <TELEMENT_TYPE, TELEMENT_SIZE, TUSAGE, mode_points>(std::move(data));
    }

    template < class TELEMENT_TYPE, size_t TELEMENT_SIZE>
    inline model_v< TELEMENT_TYPE, TELEMENT_SIZE, usage_static_draw, mode_points >
    create_model( std::vector<std::array<TELEMENT_TYPE, TELEMENT_SIZE>>&& data) const
    {
      return create_model
        <TELEMENT_TYPE, TELEMENT_SIZE, usage_static_draw>(std::move(data));
    }

    inline void use_program(const program& p) const
    {
      L(INFO,
        "--> WRP::GLEW::glew::use_program; program"
        "[" << p.program_ << "] (program object address is " << &p << ")"
      );
      C::glUseProgram(p.program_);
      WRP_GLEW_TEST_ERROR
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

