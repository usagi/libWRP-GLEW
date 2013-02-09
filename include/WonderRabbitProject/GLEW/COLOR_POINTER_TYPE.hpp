enum class COLOR_POINTER_TYPE : GL::GLenum
{ FLOAT         = GL_FLOAT
, UNSIGNED_BYTE = GL_UNSIGNED_BYTE
};

#undef GL_FLOAT
#undef GL_UNSIGNED_BYTE

std::string to_string(COLOR_POINTER_TYPE v){
  switch(v){
  case COLOR_POINTER_TYPE::FLOAT         : return "FLOAT";
  case COLOR_POINTER_TYPE::UNSIGNED_BYTE : return "UNSIGNED_BYTE";
  default                                : return "";
  }
}

