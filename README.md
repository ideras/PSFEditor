# PSFEditor (PC Screen Font Editor)

PC Screen Font Editor is an application that can open [PSF](https://wiki.osdev.org/PC_Screen_Font) version 1 and 2 files and edit them.
You can save the resulting as a MIF (Memory Initialization Format) file to initialize a ROM memory
in Verilog.

## Screenshot

![PSFEditor][psfeditor.png?raw=true]

### Compiling

To compile the application you'll need QT5. Run the following commands.

```
$ qmake
```

Then

```
make
```

## Author

* **Ivan de Jesus Deras**

## License

This project is licensed under the [BSD License](https://opensource.org/licenses/BSD-3-Clause).

## Limitations

The application is not able to save back to PSF.  Also there's no Unicode support.
