enum class ERROR : GL::GLint
{ NO_ERROR          = GL_NO_ERROR
, INVALID_ENUM      = GL_INVALID_ENUM
, INVALID_VALUE     = GL_INVALID_VALUE
, INVALID_OPERATION = GL_INVALID_OPERATION
, STACK_OVERFLOW    = GL_STACK_OVERFLOW
, STACK_UNDERFLOW   = GL_STACK_UNDERFLOW
, OUT_OF_MEMORY     = GL_OUT_OF_MEMORY
, TABLE_TOO_LARGE   = GL_TABLE_TOO_LARGE
};

#undef GL_NO_ERROR
#undef GL_INVALID_ENUM
#undef GL_INVALID_VALUE
#undef GL_INVALID_OPERATION
#undef GL_STACK_OVERFLOW
#undef GL_STACK_UNDERFLOW
#undef GL_OUT_OF_MEMORY
#undef GL_TABLE_TOO_LARGE

std::string to_string(ERROR v){
  switch(v){
  case ERROR::NO_ERROR          : return "NO_ERROR";
  case ERROR::INVALID_ENUM      : return "INVALID_ENUM";
  case ERROR::INVALID_VALUE     : return "INVALID_VALUE";
  case ERROR::INVALID_OPERATION : return "INVALID_OPERATION";
  case ERROR::STACK_OVERFLOW    : return "STACK_OVERFLOW";
  case ERROR::STACK_UNDERFLOW   : return "STACK_UNDERFLOW";
  case ERROR::OUT_OF_MEMORY     : return "OUT_OF_MEMORY";
  case ERROR::TABLE_TOO_LARGE   : return "TABLE_TOO_LARGE";
  default                       : return "";
  }
}

ERROR get_error(){ return ERROR(C::glGetError()); }

inline bool is_no_error(const ERROR& v){ return v == ERROR::NO_ERROR; }
inline bool is_error(const ERROR& v){ return ! is_no_error(v); }

#define WRP_GLEW_TEST_ERROR \
{ \
  auto e = get_error(); \
  if(is_error(e)) \
  { \
    auto message = std::string("get_error: ") + to_string(e); \
    L(ERROR, message); \
    throw std::runtime_error(message); \
  } \
}

