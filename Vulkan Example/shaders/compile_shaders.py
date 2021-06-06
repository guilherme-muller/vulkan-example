import os

## Sticking with simple now, let's just hard-code the shaders. This can be extended later for folder search and compile
## of fragment and vector shaders (if multiples)
## This script has to be configured in your pre-build configuration in your IDE
result = os.system("glslc.exe shader.frag -o frag.spv")
if (result != 0):
    exit(1)
result = os.system("glslc.exe shader.vert -o vert.spv")
if (result != 0):
    exit(1)