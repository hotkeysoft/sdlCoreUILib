# SDL GUI Library
This is a basic window-based library to build a GUI in SDL projects. It provides the following:

- Windows
  - Overlapped 
  - Widget containers
  - Move/Resize
  - Scroll bars
- Widgets
  - Label
  - Image
  - Button (with image and/or label)
  - Toolbar
  - Menu
  - Tree
- Resource Manager
  - Load internal or external resources
  - Image
  - ImageMap
  - Font
  - Cursor

## Note
This library is in no way functionally complete. It is used in some of my projects and is not actively developed or supported.

## Requirements
- Visual Studio 2019
- GUI Library
  - _SDL2_ (2.0.20) [link](https://www.libsdl.org/download-2.0.php)
  - _SDL2_image_ (2.0.3) [link](https://www.libsdl.org/projects/SDL_image/)
  - _SDL2_ttf_ (2.0.18) [link](https://github.com/libsdl-org/SDL_ttf/releases/tag/release-2.0.18)
  - Use the Visual C++ 32/64-bit Development libraries

## Fonts
The following OFL (Open Font License) fonts are packaged with the library. 

- Oxygen (Default proportional font - Regular & Bold) [link](https://fonts.google.com/specimen/Oxygen)
- FiraMono (Default Monospace font) [link](https://fonts.google.com/specimen/Fira+Mono)

External fonts can also be used.

## Configuration
  SDL Library paths are configured in `CoreUI/SDLPath.props`.
  The following variables need to be updated to match your environment:
- _SDLBase_: base directory of the _SDL2_ library
- _SDLImageBase_: base directory of the _SDL2_image_ library
- _SDLTTFBase_: base directory of the _SDL2_ttf_ library

