# SecurePhotoViewer

View encrypted archive of photos.

This was built for personal use but I thought to make the code public for anyone who is interested.

![screenshot]

## Introduction

This is a quick image viewing tool based on SFML that allows you to view images from secure archive that was encrypted using AES

The contents of archive is expected to be in following format

```
<IV>:<Base-64 of Encrypted Zip File>
```

## Build
Prerequisites:
 * [libzip](https://github.com/nih-at/libzip/blob/master/INSTALL.md)
 * [SFML 2.6.0](https://github.com/SFML/SFML/releases/tag/2.6.0)
 * C++17
 
Once you have above libraries properly installed, simply run `make` command to build binary.

## Why C++17?

The only feature used from C++17 standard is [filesystem](https://en.cppreference.com/w/cpp/filesystem) library, otherwise C++11 should be fine if you remove `createDirectory()` function or replace it with an alternative.

### Usage

In order to run program you will need to provide AES key in first 
param and archive name in second, e.g,

```
   ./secure-photo-viewer ARCHIVE KEY <INITIAL_IMAGE>
```

### Demo
To open sample archive

```
 ./secure-photo-viewer sample-archive.archive `cat sample-key`
```
   
### Keyboard

- Right Arrow: Next photo / Re-position when zoomed
- Left Arrow: Prev photo / Re-position when zoomed
- Up Arrow: Re-position when zoomed
- Down Arrow: Re-position when zoomed
- Equal: Zoom In
- Backspace: Zoom Out
- Backslash (`\`): Reset (zoom, position and rotation)
- F Key: Enter/exit Fullscreen
- Escape: Exit

 [screenshot]: https://github.com/abumq/SecurePhotoViewer/raw/master/screenshot.png?v1
