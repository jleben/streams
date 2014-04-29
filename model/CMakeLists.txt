set(src
  model.cpp
  code.cpp
  execution.cpp
)

if(WIN32)
  include_directories("C:/Program Files (x86)/Intel/OpenCL SDK/3.0/include")
  link_directories("C:/Program Files (x86)/Intel/OpenCL SDK/3.0/lib/x64")
endif()

add_executable(test ${src} test.cpp )

target_link_libraries(test OpenCL)
