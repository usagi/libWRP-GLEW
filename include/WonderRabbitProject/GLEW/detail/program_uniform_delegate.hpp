#pragma once

namespace detail
{
  template<class T, size_t N>
  class program_uniform_delegate
  {
    friend program;
    decltype(nullptr) operator()(GL::GLuint program_, GL::GLint location_);
  };

  template<>
  class program_uniform_delegate<GL::GLfloat, 1>
  {
    friend program;
    using t = GL::GLfloat;
    t operator()(GL::GLuint program_, GL::GLint location_)
    {
      t r;
      C::glGetUniformfv(program_, location_, &r);
      return std::move(r);
    }
  };
  
  template<>
  class program_uniform_delegate<GL::GLint, 1>
  {
    friend program;
    using t = GL::GLint;
    t operator()(GL::GLuint program_, GL::GLint location_)
    {
      t r;
      C::glGetUniformiv(program_, location_, &r);
      return std::move(r);
    }
  };
  
  template<>
  class program_uniform_delegate<GL::GLuint, 1>
  {
    friend program;
    using t = GL::GLuint;
    t operator()(GL::GLuint program_, GL::GLint location_)
    {
      t r;
      C::glGetUniformuiv(program_, location_, &r);
      return std::move(r);
    }
  };
  
  template<>
  class program_uniform_delegate<GL::GLdouble, 1>
  {
    friend program;
    using t = GL::GLdouble;
    t operator()(GL::GLuint program_, GL::GLint location_)
    {
      t r;
      C::glGetUniformdv(program_, location_, &r);
      return std::move(r);
    }
  };
  
  template<size_t N>
  class program_uniform_delegate<GL::GLfloat, N>
  {
    friend program;
    using t = GL::GLfloat;
    using rt = vec<t, N>;
    rt operator()(GL::GLuint program_, GL::GLint location_)
    {
      rt r;
      C::glGetUniformfv(program_, location_, r.data());
      return std::move(r);
    }
  };
  
  template<size_t N>
  class program_uniform_delegate<GL::GLint, N>
  {
    friend program;
    using t = GL::GLint;
    using rt = vec<t, N>;
    rt operator()(GL::GLuint program_, GL::GLint location_)
    {
      rt r;
      C::glGetUniformiv(program_, location_, r.data());
      return std::move(r);
    }
  };
  
  template<size_t N>
  class program_uniform_delegate<GL::GLuint, N>
  {
    friend program;
    using t = GL::GLuint;
    using rt = vec<t, N>;
    rt operator()(GL::GLuint program_, GL::GLint location_)
    {
      rt r;
      C::glGetUniformuiv(program_, location_, r.data());
      return std::move(r);
    }
  };
  
  template<size_t N>
  class program_uniform_delegate<GL::GLdouble, N>
  {
    friend program;
    using t = GL::GLdouble;
    using rt = vec<t, N>;
    rt operator()(GL::GLuint program_, GL::GLint location_)
    {
      rt r;
      C::glGetUniformdv(program_, location_, r.data());
      return std::move(r);
    }
  };
}

