# nemesisOS

nemesisOS is an original mobile operating system project. Its first target is a from-scratch AArch64 kernel, `nemesisKernel`, initially developed and tested on QEMU's `virt` machine before hardware bring-up on Google Pixel 10.

## Current milestone

- AArch64 bare-metal kernel
- UART output
- CPU exception handling
- Generic timer and delays
- Round-robin task scheduler
- Physical memory allocator
- virtio-gpu framebuffer driver
- Graphical boot splash and loading animation

## Build

Install an AArch64 cross-toolchain and QEMU:

```sh
sudo apt install gcc-aarch64-linux-gnu qemu-system-arm
```

Build:

```sh
make
```

Run in terminal mode:

```sh
make run
```

Run with a graphical QEMU window:

```sh
make run-gui
```

Expected UART output:

```text
nemesisOS v0.0.3
nemesisKernel: graphics bring-up
virtio-gpu: framebuffer ready
Starting scheduler...
```

With `make run-gui`, QEMU should open a window showing the nemesisOS boot splash and animated loading bar.

## Repository layout

```text
nemesisKernel/
  arch/arm64/       AArch64 startup, timer, exceptions, scheduler switch
  kernel/           Core kernel services
  mm/               Physical memory management
  drivers/gpu/      virtio-gpu framebuffer driver
  include/          Public kernel headers
  ipc/              Inter-process communication (future)
  fs/               File systems (future)
  net/              Networking (future)
  security/         Security subsystem (future)
```

## License

MIT. See `LICENSE`.
