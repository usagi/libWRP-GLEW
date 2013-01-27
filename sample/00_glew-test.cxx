#include <iostream>
#include <glog/logging.h>

#include <WonderRabbitProject/GLEW.hpp>

namespace
{
  using namespace std;
  using namespace WonderRabbitProject;

  constexpr auto name = "00_glew-test";
  
  enum class GLOG_OUT
  {
    DEFAULT,
    STDERR
  };
  
  void glog_initialize(const GLOG_OUT out)
  {
    google::InitGoogleLogging(name);
    switch(out)
    {
    case GLOG_OUT::STDERR:
      google::LogToStderr();
    default:;
    }
    LOG(INFO) << "glog initialized";
  }
}

int main() try
{
  glog_initialize(GLOG_OUT::STDERR);
}
catch (const std::exception& e)
{ std::cerr << "exception: " << e.what(); }
catch (...)
{ std::cerr << "unknown exception"; }

