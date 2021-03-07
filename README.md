# MBR Snake 🐍
![Version](https://img.shields.io/badge/Version-1.0-blue.svg) ![License](https://img.shields.io/badge/License-MIT-green.svg)


## Intro 🖊️


A classic game of Snake that fits into an x86 boot sector, written in C++17.

The main goal of this project is to show how it's possible to use modern tools such as STL algorithms, lambdas, templates and more, to target even the most low-level environments without any runtime overhead. [This CppCon talk](https://www.youtube.com/watch?v=zBkNBP00wJE) by Jason Turner with a similar theme was a big inspiration behind making this.


![Gameplay](https://i.imgur.com/yP49Wzl.gif)


An x86 boot sector has 510 bytes of usable program space. The entire game including logic, graphics, input handling etc. has to fit in that space. Since there's no underlying OS to provide further runtime functionality (like memory allocation), the binary has to be fully self-sufficient as well.

Originally I developed a version of this for a university assignment in 2017, however, at the time I had to cut some features due to the size limit. Recently I stumbled across that original project and decided to give it another go. This time I was finally able to fully implement the game in under 510 bytes, so I figured I'd upload it here just for the snake of it.

In the code you might see some odd choices in the way I did things, but the explanation is almost always the executable size. There were a few places where writing something in a slightly unorthodox way resulted in a decreased binary size, so if you see anything odd, that's probably why.


## Usage 🖥️


1. To run the game, you can **download a bootable image from [Releases](https://github.com/adam10603/mbr_snake/releases)**, or build one yourself (see [Building](#building-%EF%B8%8F)).

2. After you have an image, you can **use a virtual machine** to boot it (such as VirtualBox or QEMU), or even a real PC by flashing the image onto a USB drive for example.

    - **[This browser-based VM](https://copy.sh/v86/) is very convenient**. Open the site, select `mbr_snake.img` as "Floppy disk image", and click "Start emulation". I suggest increasing the "Scale" setting to see things better.

3. Once you've booted the game, you can play using the **arrow keys** ⬆ ➡ ⬇ ⬅ .


## Building 🛠️


Building this project **specifically requires GCC on Linux**. Your mileage may vary depending on your GCC version though, since a different version may produce a slightly oversized binary. For reference, I used **GCC 9.3**.

1. Make sure `libc6-dev-i386` or your distro's equivalent 32-bit dev library is installed. [See this post](https://stackoverflow.com/a/7412698/3606363) for more info.

2. Use `make` for the initial build. This creates one file in the `bin` directory:

    - `mbr_snake.bin` - **This is the raw program binary** and nothing else. It's useful for checking the final size of the program (cannot be over 510 bytes).

3. Use `make image` which uses the file from step 2 to make two more files (these are what you get in a [Release](https://github.com/adam10603/mbr_snake/releases)):

    - `mbr_snake_bs.bin` - Same as the file from step 2, but it's been padded to 512 bytes and given the 2-byte MBR signature `55 AA` at the end. **This is an exact image of a boot sector**.

    - `mbr_snake.img` - This is the final product you'll probably use, as **this is a bootable floppy disk image**. It's a 1.44MB zero-filled file with the contents of `mbr_snake_bs.bin` copied to its first 512 bytes, making for an empty floppy disk image with the game in its boot sector.


## Known Issues 🛑


- A newly spawned food item can overlap with the snake's body. This isn't game-breaking though, as you can just move over a bit, then come back to grab it. Sadly, inserting a check for this would take up too many instructions, and there's not enough space for that.

- Pressing specifically `End` or `PageUp` will temporarily break the game until you press an arrow key. Once again, the reason for this is the lack of binary space for doing more extensive input checking. As to why those two keys in particular are the problem, that comes down to keyboard scan code values.


## Version history 📃


* v1.0
  * Initial release


_____________________
![MIT Logo](https://upload.wikimedia.org/wikipedia/commons/thumb/0/0c/MIT_logo.svg/32px-MIT_logo.svg.png) Licensed under [MIT License](LICENSE).
