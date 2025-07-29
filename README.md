# GsTicKy

A simple GTK 4 desktop application for creating and managing sticky notes.

## Features

- Create, edit, and save notes
- Change note background color using a dropdown
- Configurable color themes
- Persistent note storage
- Keyboard shortcuts for quick access

## Build & Run

### Prerequisites

- GTK 4 development libraries
    - `libgtk-4-dev`
- Make

### Build

```sh
make
```

### Windows Cross-Compilation (Linux → Windows)

To build for Windows from a Linux system:

#### Prerequisites
- MinGW-w64 cross-compiler: `x86_64-w64-mingw32-gcc`
- GTK 4 Windows development libraries

#### Setup GTK for Windows
1. **Download GTK libraries** using one of these methods:
   - **MSYS2 approach**: Follow [GTK Windows installation guide](https://www.gtk.org/docs/installations/windows#using-gtk-from-msys2-packages)
   - **Direct download**: Get pre-built GTK libraries for Windows

2. **Install libraries** to `/opt/gtk-win64/`:
   ```sh
   # If using MSYS2, copy from Windows installation:
   # C:\msys64\mingw64\[bin,include,lib] → /opt/gtk-win64/
   ```

3. **Build Windows executable**:
   ```sh
   make win          # Build Windows .exe
   make dist-win     # Create distribution with required DLLs
   ```

The distribution will be created in `dist/` containing the executable and minimal set of required GTK DLLs.

### Run

```sh
./build/sticky-notes-app
```

## File Structure

- `src/` — Source code
- `res/` — UI files (`main.ui`, `config.ui`)
- `include/` — Header files

## Usage

- Launch the app to start a new note.
- Use the config window to change background color.
- Notes are saved to the location in the top line of each file.

## License

MIT License

---

**Contributions welcome!**