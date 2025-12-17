# Core Microkernel Operating System

A fully functional microkernel operating system written in C/C++ for x86_64 architecture.

## Features

- **Microkernel Architecture**: Minimal kernel with services in user space
- **Memory Management**: Physical/Virtual memory with buddy allocator
- **Process Management**: Multi-process with scheduling
- **IPC**: Message passing and shared memory
- **Device Drivers**: VGA console, PCI, timers
- **x86_64 Support**: Full 64-bit implementation

## Building
```bash
# Install dependencies
sudo apt-get install build-essential nasm qemu-system-x86 xorriso grub-pc-bin

# Build kernel
make all

# Create ISO
make iso

# Run in QEMU
make run
```

## Documentation

See `docs/` directory for detailed documentation.

## License

MIT License - see LICENSE file
```

### `LICENSE`
```
MIT License

Copyright (c) 2024 Core Kernel Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
