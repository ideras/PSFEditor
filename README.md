# PSFEditor (PC Screen Font Editor)

PC Screen Font Editor is an application that can open [PSF](https://wiki.osdev.org/PC_Screen_Font) version 1 and 2 files and edit them.
You can save the resulting as a MIF (Memory Initialization Format) file to initialize a ROM memory in Verilog.

## Screenshot

![PSFEditor](psfeditor.png?raw=true)

### Compiling

To compile the application you'll need QT5. Run the following commands.

```
$ qmake
```

Then

```
make
```

### Nix

Providing flake.nix

```bash
nix run github:ideras/PSFEditor
```

## Author

* **Ivan de Jesus Deras**

## License

This project is licensed under the [BSD License](https://opensource.org/licenses/BSD-3-Clause).

## Features

* Open/Save PSF version 1 and 2. Thanks to [psftools](https://tset.de/psftools/index.html) by Gunnar Zötl
* Open/Save fonts in Verilog MIF files. These files are used to initialize memories.
