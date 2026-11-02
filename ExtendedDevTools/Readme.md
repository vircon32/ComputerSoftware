# Vircon32: Extended development tools

This folder contains submodules for 3 external repositories with additional development tools for Vircon32. These are 3 very relevant tools for Vircon32 development made by **Matthew Haas**. We have been testing this tools in close contact for some time now, so you could consider these as "**second party tools**".

## v32lua

- [v32lua repository](https://github.com/wedge1020/v32lua)

This is a **Lua compiler for Vircon32**. That's right, you can write Vircon32 games in Lua now! The repository contains extensive documentation and examples on how to use it. This compiler implements a native lua API to make Vircon32 games, but is also able to compile PICO-8 and TIC-80 cartridges (even automatically converting the original game assets).

## v32c++

- [v32cxx repository](https://github.com/wedge1020/v32cxx)

This is a transpiler that will **translate C++ and standard C code**, into the limited C variant that the Vircon32 C compiler uses. This will allow you to write Vircon32 games in regular C, or even in C++. On C++ there are some limitations since the language is huge but you can use the basics like classes, methods, strings, etc.

## v32opt

- [v32opt repository](https://github.com/wedge1020/v32opt)

This is an **optimizer that can be applied directly over assembly code**, so it can potentially be used for games written in any language (C, C++, Lua or Assembly itself). It can be pretty useful, since the Vircon32 C compiler does almost no optimization. Same as other tools, it has different optimization levels (-O1, -O2, -O3) with the usual trade-offs: more aggresive optimizations are more likely to get relevant speed boosts, but are also more likely to break some part of the program.

### License

At the time of writing, those 3 repositories use the same general license as this one: the **3-Clause FreeBSD License**. Still, do check the original repositories for documentation, licences and other details since they might change.

### How these submodules work

In general, within this repository all 3 submodules will be kept **fixed to a specific release tag** instead of tracking the latest changes in master. That way it becomes possible to use the same release binaries as in the original repositories instead of doing the builds again.

To download these tools after cloning this repository, run this command from its root folder:

```sh
git submodule update --init --recursive
```

Alternatively, you can include `--recurse-submodules` when cloning this repository.

See each tool's own documentation for build instructions and usage.
