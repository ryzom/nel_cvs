*************
*WorldEditor*
*************

Keys
----

T : Toggle mode zone/transition
G : Toggle grid on/off
Crtl-Z / Ctrl-Y : Undo / Redo the last action on the current mode (Prim or Land)
Space Or Middle Button + MouseMove : Move the current view window
Left Button : put ligozone or add/move primitive vertices
Right Button : delete
Middle Button Roll Or Shift Mouse move : zoom in/out


Remarks
-------

RootDirectory is the directory where the WorldEditor is executed.
The LigoZone files must be stored in the subdirectory named "ZoneLigos" just under 
the RootDirectory. The TGA files that are snapshots of the zones must go in the
subdirectory named "ZoneBitmaps".


LigoZone
--------

A LigoZone is a file that describe a .zone in term comprehensible by the worldEditor.
This is due to the fact that worlEditor do not load .zone. It has just a 2D representation
and so need a .ligozone and a .tga (snapshot of the zone). A Ligo zone can be a simple zone
of (cellsize x cellsize) or a meta zone of (n*cellsize x m*cellsize). The metazone has a
mask to know where the unitzones of the metazone are.
All this data are stored in a ligozone file in XML.


Land
----

The .land is the extension for landscape legoscape representation. The files.land are files
that regroups ligozone together to make a region. This file is used in the export process to
make zones loaded in the client game application.


Prim
----

The .prim represents the logic (a group of concave shapes, lines and vertices).
It is associated with form in georges to describe content (vegetable, constructions, etc.)
Because they have no sense alone there is no special export for .prim.


Export
------

The export process need 
- The .Land to know what zone to export
- The reference zone (file .zone) given by the max plug-in.
- A Tile bank file to rotate and flip tiles when the reference zone is flipped.
- An optionnal height map which is scaled over the whole world (coordinated [0,0] to
  [256*CellSize, -256*CellSize]. So if you want a precision of 16m with a cellsize of 160m
  you will have a height map of 2560x2560. The height map is a tga in grayscale. 
  The level 127 represents the deltaZ=0, 255 -> deltaZ=+128*ZFactor and 0 -> deltaZ=-127*ZFactor.
  The ZFactor is the multiplication to apply to each level of the height map.
  A ZFactor of 1 means that each level represents 1 meter of deltaZ.
