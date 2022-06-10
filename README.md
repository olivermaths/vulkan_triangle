# simple program that display a triangle on the screen.

the meaning of this project is just to learn how vulkan works.

### Running the project.
open the project directory, then run:
```
glslc src/triangle.vert -o vert.spv
glslc src/triangle.frag -o frag.spv

clang -Wall -pedantic src/main.c -lGL -lglfw -lvulkan -o bin/engine

bin/engine
```

![alt](https://raw.githubusercontent.com/olivermaths/vulkan_triangle/main/triangle.jpeg)
