# learning opengl
this is repository with my opngl expirience. there is some bad code and this is a mess.

# building
uses cmake:
``` shell
cmake -S . -B build
cmake --build build
build/main.exe
```

if you dont know what the shell is, then you could use launcher that i made. [get it here](github.com/NikitaWeW/launcher/releases/latest)
create any file and paste following configuration in there:
``` json
{
    "repo": "https://github.com/NikitaWeW/graphic-setup.git",
    "branch": "dev",
    "executable": "build\\main.exe",
    "additional packages": [
        "python",
        "python-jinja"
    ]
}
```

then, open that file with configuration with launcher executale. you can drag configuration file in launcher or right click it -> open with -> path/to/launcher.exe. 

detailed instructions [here](https://github.com/NikitaWeW/launcher/blob/main/README.md).