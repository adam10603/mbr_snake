# MBR Snake
![Version](https://img.shields.io/badge/Version-1.0-blue.svg) ![License](https://img.shields.io/badge/License-MIT-green.svg)


## Intro


A classic game of Snake that runs from an x86 boot sector, written in C++17.

The main goal of this project is to show how you can use modern tools such as STL algorithms, lambdas, templates and more, to target even the most low-level environments with zero runtime overhead. [This 2016 CppCon talk](https://www.youtube.com/watch?v=zBkNBP00wJE) by Jason Turner with a similar theme was a big inspiration behind making this.

Originally I developed a version of this for a university assignment in 2017, however, at the time I couldn't fully implement every feature of the game due to the size restrictions of a boot sector. Recently in 2021 I stumbled across that original project and decided to give it another go. Thanks to everything I've learned since then, I was able to fully implement the game this time, so I decided to upload it here for the sake of it.


## Usage


1. To run the game, you can **download a bootable floppy image from [Releases](https://github.com/adam10603/mbr_snake/releases)**, or build one yourself (see [Building](#building)).

2. After you have an image, you can **use a virtual machine** to boot it (such as VirtualBox or QEMU), or even a real PC by flashing the image onto a USB drive for example.

  - **[This browser-based VM](https://copy.sh/v86/) is probably the easiest**. Simply go there, upload the image under "Floppy disk image", and click "Start emulation". I suggest increasing the "Scale" setting to see things better.

3. Once you've booted the game, you can play using the **arrow keys** ⬆ ➡ ⬇ ⬅ .


## Building


This project **specifically requires GCC on Linux** to be built. Your mileage may vary depending on your GCC version though, since a different version may produce a binary that's too large. For the record, I used **GCC 9.3**. 

1. Make sure you have `libc6-dev-i386` or your distro's equivalent 32-bit dev library installed. [See this post](https://stackoverflow.com/a/7412698/3606363) for more information.

2. Use `make` for the initial build. This creates one file in the `bin` directory:
  - `mbr_snake.bin` - **This is the raw program binary** and nothing else. It's useful for checking the final size of the program (cannot be over 510 bytes).

3. Use `make image` which uses the file from step 2 to produce the following files:

  - `mbr_snake_bs.bin` - Its contents are the same as the file from step 2, but its been padded to exactly 512 bytes and given the 2-byte MBR signature `55 AA` at the end. **This is an exact image of a boot sector**.

  - `mbr_snake.img` - This is the final product you'll probably use, as **this is a bootable floppy disk image**. It's a 1.44MB zero-filled file with the contents of `mbr_snake_bs.bin` copied to its first 512 bytes, creating an empty floppy disk image with the game in its boot sector.


## Known Issues


- A newly spawned food item can overlap with the snake's body. This isn't game-breaking though, as you can just move over a bit, then come back to grab it. Sadly, inserting a check for this would take up too many instructions, and there's not enough space for that.

- Pressing specifically `End` or `PageUp` will temporarily break the game until you press an arrow key. Once again, the reason for this is the lack of binary space for doing more extensive input checking. As to why those two keys in particular are the problem, that comes down to keyboard scan code values.


## Version history


* v1.0
  * Initial release


_____________________
![MIT Logo](https://upload.wikimedia.org/wikipedia/commons/thumb/0/0c/MIT_logo.svg/40px-MIT_logo.svg.png) Licensed under [MIT License](LICENSE).