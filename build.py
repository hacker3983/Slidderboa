import os
import platform

LINK_FILES = "-lSDL2main -lSDL2 -lSDL2 -lSDL2_ttf -lSDL2_mixer -lSDL2_image"

C_FILES = [
    "slidderboa.c",
    "slidderboa_assetmanager.c"
]
O_FILES = " ".join(C_FILES)

print("Building Slidderboa...")
os.system(f"gcc -c {' '.join(C_FILES)}")
print("Running Slidderboa...")
if platform.system() == "Windows":
    os.system(f"gcc main.c {O_FILES} -lmingw32 {LINK_FILES} -o slidderboa && .\\slidderboa")
else:
    os.system(f"gcc main.c {O_FILES} {LINK_FILES} -o slidderboa && ./slidderboa")