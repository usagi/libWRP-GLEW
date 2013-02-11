#pragma once

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

