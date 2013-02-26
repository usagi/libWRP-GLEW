#pragma once

#include "./detail/model_invoke_vertex_attribute_pointer.hpp"

struct model
{
  friend glew;
  virtual ~model() { }
protected:
  virtual void invoke() const = 0;
};

template
<
  class TVERTEX,
  class TUSAGE,
  class TDEFAULT_INVOKE_MODE
>
struct model_v final : model
{
  friend glew;
  using vertex_type = TVERTEX;
  using data_type = std::vector<vertex_type>;
  static constexpr USAGE usage = USAGE(TUSAGE::value);
  static constexpr MODE default_invoke_mode = TDEFAULT_INVOKE_MODE::value;
public:
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
      vertex_type::size * vertices,
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
    detail::model_invoke_vertex_attribute_pointer<vertex_type>::invoke();
    C::glDrawArrays(GL::GLenum(TMODE::value), 0, vertices);
    C::glBindBuffer(GL_ARRAY_BUFFER, 0);
    //C::glBindVertexArray(0);
  }
  
  void invoke() const override
  { invoke<TDEFAULT_INVOKE_MODE>(); }
};

template
<
  class TVERTEX,
  class TUSAGE,
  class TDEFAULT_INVOKE_MODE
>
struct model_vi final : model
{
  friend glew;
  using vertex_type = TVERTEX;
  using data_vertices_type = std::vector<vertex_type>;
  static constexpr USAGE usage = USAGE(TUSAGE::value);
  static constexpr MODE default_invoke_mode = TDEFAULT_INVOKE_MODE::value;
  using data_indices_element_type = GL::GLuint;
  using data_indices_type = std::vector<data_indices_element_type>;
public:
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
      vertex_type::size * num_of_vertices,
      data_vertices.data(),
      GL::GLenum(usage)
    );
    C::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer());
    C::glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      sizeof(data_indices_element_type) * num_of_indices,
      data_indices.data(),
      GL::GLenum(usage)
    ); 
    C::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    C::glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  
  template<class TMODE>
  void invoke() const override
  {
    C::glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer());
    C::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer());
    detail::model_invoke_vertex_attribute_pointer<vertex_type>::invoke();
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

