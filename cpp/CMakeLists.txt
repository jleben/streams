#add_executable(run main.cpp)

set(compiler_src
  compiler/graph.cpp
  compiler/code.cpp
  compiler/kernel.cpp
)

#add_executable(test_compiler ${compiler_src} test_compiler.cpp )
#add_executable(test1 ${compiler_src} test_compile1.cpp )

if(WIN32)
  include_directories("C:/Program Files (x86)/Intel/OpenCL SDK/3.0/include")
  link_directories("C:/Program Files (x86)/Intel/OpenCL SDK/3.0/lib/x64")
endif()

add_executable(test-run ${compiler_src} compiler/test.cpp )

target_link_libraries(test-run OpenCL)
