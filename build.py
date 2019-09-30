import os
import time
import shutil
import platform

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

if platform.system() == "Linux":
    shutil.copy(f"./libdiscord_pp.a", "/usr/lib/libdiscord_pp.a")
    shutil.copytree("../include", "/usr/include/discord_pp")
    print("Moved binary and includes to /usr/lib and /usr/include")


print("Total compilation time:", round(time.time() - before, 2), "seconds")
