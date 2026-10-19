# nemesisOS

nemesisOS is an original mobile operating system project. Its first target is a from-scratch AArch64 kernel, `nemesisKernel`, initially developed and tested on QEMU's `virt` machine before hardware bring-up on Google Pixel 10.

## Current milestone

- AArch64 bare-metal kernel skeleton
- QEMU PL011 UART output
- Basic panic handling and formatted printing
- QEMU build and run workflow

## Build

Install an AArch64 bare-metal toolchain and QEMU:

```sh
# Debian/Ubuntu example
sudo apt install gcc-aarch64-linux-gnu qemu-system-arm
```

Build:

```sh
make
```

Run in QEMU:

```sh
make run
```

Expected output:

```text
nemesisOS v0.0.1
nemesisKernel: AArch64 bring-up
```

## Repository layout

```text
nemesisKernel/
  arch/arm64/       AArch64 startup, UART, early platform code
  kernel/           Core kernel services
  include/          Public kernel headers
  drivers/          Device drivers (future)
  mm/               Memory management (future)
  ipc/              Inter-process communication (future)
  fs/               File systems (future)
  net/              Networking (future)
  security/         Security subsystem (future)
```

## License

MIT. See `LICENSE`.
