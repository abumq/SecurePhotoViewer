# SecurePhotoViewer

View encrypted archive of photo

This is for personal use but I thought to make the code public for anyone who is interested.

## Introduction

This is a quick image viewing tool based on SFML that allows you to view images from secure archive that was encrypted using AES

The contents of archive is expected to be in following format

```
<IV>:<Base-64 of Encrypted Zip File>
```

## Build
Prerequisites:
 * [libzip](https://github.com/nih-at/libzip/blob/master/INSTALL.md)
 * [SFML 2.4.2](https://www.sfml-dev.org/download/sfml/2.4.2)
 
Once you have above libraries properly installed, simply run `make` command to build binary.

### Usage

In order to run program you will need to provide AES key in first 
param and archive name in second, e.g,

```
   ./secure-photo-viewer ARCHIVE KEY <INITIAL_IMAGE>
```

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
