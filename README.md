# ancient-3d-for-turboc

These source files were last modified in 1996, over 20 years ago. The target platform is Borland Turbo C for MS-DOS. I wrote them when I was 16 based on explanations in the PC Game Programmer's Encyclopaedia. They are lacking in comments and written in an oversimplified antique style. I've not attempted to compile them and something vital is probably missing.

"Glib" here is nothing to do with GNU's library of the same name. It's a set of routines for using 320x200 256 colour VGA mode graphics. It might be double-buffered, I can't remember.

The 3D part of the program does not use quaternions, instead using pre-computed Euler angle matrices. The "scene" is considered as a set of polygons, sorted by depth, then painted far-to-near with visibility culling. Lighting model is flat shading with a single light and no texture mapping. I remember this achieved a few thousand polygons per second on a 33MHz 486DX typical of the era.
