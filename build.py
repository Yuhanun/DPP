import os
import time

use_clang = True

if not os.path.isdir("build"):
    os.mkdir("build")

os.chdir("build")

before = time.time()

cmake_command = "cmake .."
if use_clang:
    cmake_command += " -DCMAKE_CXX_COMPILER=clang++"

if not os.system(cmake_command) == 0:
    exit()

if not os.system("make") == 0:
    exit()

print("Total compilation time:", round(time.time() - before, 2), "seconds")
