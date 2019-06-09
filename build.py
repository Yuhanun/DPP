import os

if not os.path.isdir("build"):
    os.mkdir("build")

os.chdir("build")

if not os.system("cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++") == 0:
    exit()

if not os.system("make") == 0:
    exit()