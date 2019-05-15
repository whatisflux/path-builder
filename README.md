# Path Builder
Code for the algorithm that turns a bitmap representing where the path is to a data structure explicitly defining the path.

## Usage

Install correct version of SFML, put it in this directory and name it SFML-2.5.1.

Move `sfml-graphics-2.dll`, `sfml-system-2.dll`, and `sfml-window-2.dll` to the main folder

Run `make` to build the project. It outputs `test.exe`. `make clean` will remove temporary files.

## udp-img

Run `make udp-img` to build the executable that will process and display an image from a udp server.

To send an image to the server, send the following data to port 1234:
- A 80x60 (width x height) bitmap represented as the bits in characters.

## Path Data Structure
A `Path` contains 2 `Edges`, `edge1` and `edge2`.

An `Edge` contains an `std::vector<pb::Waypoint> waypoints` listing the points in order for the class. Also, if `Edge.isClosed`, the edge is a loop (first and last `Waypoints` are connected).

A `Waypoint` contains a `pb::Vector position` representing the floor-space position and a `bool insideIsLeft` describing which side the inside of the path is on.

### Serialized version
A `Path` is serialized as:
- Serialization of `edge1`
- Serialization of `edge2`

An `Edge` is serialized as:
- The character `e`
- Serialization of all `Waypoints` it contains
- The character `z` if `isClosed`
    - Otherwise, ends when next `e` found or string ends

A `Waypoint` is serialized as:
- Begins with a `p`
- Serialization of `position.x`
- Serialization of `position.y`
- `0b11111111` as a char if `insideIsLeft`, otherwise `0b00000000`
- Ends after these 10 characters
    - `p[4byte-float][4byte-float][1byte-bool]`

A `float` is serialized as:
- Raw bits of the float are packed into 4 characters