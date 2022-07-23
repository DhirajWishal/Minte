# Minte

Minte (pronounced as "mint") is a low level, Object-Oriented GUI library based on modern C++ for game UI. This uses a layered system to render the UI, where the user's image can be given as a background image and the
UI is drawn on top of that. Additionally the user can submit other images in different ways. The library will render the UI using a backend (Vulkan at least for now) and return the rendered image in real-time or as a
future.

## How to set up?

Start by cloning the repository to a local directory and initialize the submodules.

```bash
git clone https://github.com/DhirajWishal/Minte
cd Minte
git submodule init
git submodule update
```

### Pre-build

If you wish to use the library as a pre-built, then go ahead and compile the `Minte` project using CMake. Make sure to set the include directory under `{CLONED DIR}/Include` and link against the `Minte` static library.

### Using with CMake

If your project uses CMake, you can also add this project as a subdirectory and link against the `Minte` target.

## Is it cross-platform?

It will be as soon as C++20 is stable on other platforms. For now, the only thing holding us back is the compiler support. Until then, we're gonna stick with Windows.

## License

This project is licensed under MIT.
