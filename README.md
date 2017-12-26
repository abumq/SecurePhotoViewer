# SecurePhotoViewer

View encrypted archive of photo

This is for personal use but I thought to make the code public for anyone who is interested.

## Introduction

This is a quick image viewing tool based on SFML that allows you to view images from secure archive that was encrypted using AES

The contents of archive is expected to be in following format

```
<IV>:<Base-64 of Encrypted Zip File>
```

### Usage

In order to run program you will need to provide AES key in first 
param and archive name in second, e.g,

```
   ./secure-photo-viewer AES_KEY ARCHIVE <INITIAL_IMAGE>
```
   
### Keyboard

- Right Arrow: Next photo / Re-position when zoomed
- Left Arrow: Prev photo / Re-position when zoomed
- Up Arrow: Re-position when zoomed
- Down Arrow: Re-position when zoomed
- Equal: Zoom In
- Backspace: Zoom Out
- Backslash (\): Reset Zoom
- F Key: Enter/exit Fullscreen
