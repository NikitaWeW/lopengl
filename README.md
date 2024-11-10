# idontknowhowtocallitsorry
that is setup for graphic programming / learning projectthat i use. dont expect it to work. lol.

# building
uses cmake:
``` shell
cmake -S . -B build
cmake --build build
build/main.exe
```

if you dont know what shell is, then you could use launcher that i made. [get it here](github.com/NikitaWeW/launcher/releases/latest)
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

then, open that file with configuration with launcher executale. detailed instructions [here](https://github.com/NikitaWeW/launcher/blob/main/README.md).