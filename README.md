# Vroom Engine

Geometry visualizer and base for a Game Engine.
[Github link](https://github.com/KaiCaire/vroom-engine)

## Description

Including an orbital camera and a basic editor for easier use, this version consists of a 3D geometry visualizer, capable of importing different models (via drag and drop).

## Getting Started

### Camera Controls

* Free Look (WASDQE) is enabled by holding the Right Mouse Button.
* Move Forward/Backward: W / S.
* Move Left/Right: A / D.
* Move Up/Down: E / Q.
* Accelerate Movement: Hold LShift.
* Orbital Rotation: ALT + Left Mouse Button.
* Panning: ALT + Middle Mouse Button (Scroll Wheel Click).
* Zoom in Depth: ALT + Right Mouse Button.
* Scroll Zoom: Use the Mouse Wheel.
* Focus on Selected Object: Press F.

### GUI and Settings

* File -> Exit: Exit the program
* View: Toggle on/off viewing different GUI windows
    * Console: showcases all LOG messages and possible errors
      * -> Clear: clears all available LOG messages
      * -> Go to Bottom: automatically scrolls to the bottom of the console to see the latest message
    * Configuration: showcases current FPS, hardware information, memory consuption information and versions for utilized software
      * -> Fullscreen: toggle fullscreen mode on/off
      * -> Resolution: pick a window resolution
    * Hierarchy: showcases current scene's game objects
      * By clicking on the expandable arrows, children of a game object are shown/hidden
      * Clicking on a particular game object makes its information available on the Inspector window (see below)
      * Right click on a selected game object displays the Delete option in order to delete a game object
      * -> Create...: create a new game object from available options
        * -> Empty: create an empty game object
        * -> Cube: create a cube game object
    * Inspector: showcases a game object's information once it has been selected from the Hierarchy window
      * -> Game Object Name: by typing in this space, the game object's name will be changed
      * Transform: displays the game object's position, rotation and scale values
      * Mesh: displays the game object's vertices and indices counts
        * -> Show Vertex Normals: displays (or turns off display of) the game object's vertex normals
        * -> Show Face Normals: displays (or turns off display of) the game object's face normals
      * Texture: shows game object's texture's path, width and height
* Help
  * -> Documentation: opens this ReadMe
  * -> Report a Bug: opens the project's [Issues](https://github.com/KaiCaire/vroom-engine/issues) page
  * -> Latest Release: opens the project's [Releases](https://github.com/KaiCaire/vroom-engine/releases) page
  * -> About: opens window with information about the program

### Additional Actions

* que se puede hacer drag and drop y tal

## Additional Features

* cambiar el nombre, borrar objetos etc etc

## Authors

* Ivan Alvarez -> [ivalpe](https://github.com/Ivalpe)
* Kai Caire -> [KaiCaire](https://github.com/KaiCaire)
* Lara Guevara -> [LaraGuevara](https://github.com/LaraGuevara)
* Bernat Loza -> [BerniF0x](https://github.com/BerniF0x)

## Created Using

* [las mil millones de librerias aqui pls este link es inutil de momento pero darle click para easter egg](https://www.youtube.com/watch?v=OnGQeVTt4KI)

