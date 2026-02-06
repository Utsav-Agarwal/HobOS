# HobOS 

<p align="center"> <img alt="image" src="https://i.ibb.co/k21hynVd/hobos.webp" width="25%"/> </p>


<p align="center">
  <a href="https://github.com/Utsav-Agarwal/HobOS/actions/workflows/ci.yml">
    <img alt="CI" src="https://github.com/Utsav-Agarwal/HobOS/actions/workflows/ci.yml/badge.svg"/>
  </a>
  <img src="https://img.shields.io/github/languages/code-size/Utsav-Agarwal/HobOS"/>
  <img src="https://img.shields.io/github/stars/Utsav-Agarwal/HobOS"/>
  <img src="https://img.shields.io/github/forks/Utsav-Agarwal/HobOS"/>
  <a href="https://github.com/Utsav-Agarwal/HobOS/blob/main/LICENSE">
    <img src="https://img.shields.io/github/license/Utsav-Agarwal/HobOS"/>
  </a>
  <img src="https://img.shields.io/github/issues/Utsav-Agarwal/HobOS"/>
  <img src="https://img.shields.io/github/issues-pr/Utsav-Agarwal/HobOS"/>
</p>



<hr/>

HobOS is a small hobby operating system for AArch64 Raspberry Pi hardware, currently developed and tested on an RPi3B machine model under QEMU. The project is focused on learning by building, with features added and refined incrementally.


<hr/>

## Key features

- [x] AArch64 boot
- [x] Exceptions and IRQ handling
- [x] Serial console output (UART + `kprintf()` support)
- [x] MMIO and GPIO
- [x] Timer and interrupts supported
- [x] MMU and page table support
- [x] Physical memory allocation via buddy allocator
- [x] SMP support
- [ ] Userspace shell

## Getting Started

To get started, follow these steps:

1) Clone the repo
```bash
git clone https://github.com/Utsav-Agarwal/HobOS.git
cd HobOS
```

2) Install dependencies (Debian/Ubuntu)
```bash
make install
```

3) Build
```bash
make
```

4) Run (QEMU, RPi3B model)
```bash
make run
```

Optional: debug run
```bash
make run_dbg
```

## Contributing

Issues and pull requests are welcome. If you’re making a larger change, consider opening an issue first to discuss it.
