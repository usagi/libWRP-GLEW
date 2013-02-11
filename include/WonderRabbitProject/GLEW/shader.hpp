#pragma once

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

