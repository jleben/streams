#add_executable(run main.cpp)

set(compiler_src
  compiler/graph.cpp
  compiler/code.cpp
  compiler/kernel.cpp
)

#add_executable(test_compiler ${compiler_src} test_compiler.cpp )
add_executable(test1 ${compiler_src} test_compile1.cpp )
