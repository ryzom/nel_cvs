************
Master Tools
************

This tools regroup the 3 other tools in a workspace way.
- WorldEditor : Easily generates landscapes and primitives
- LogicEditor : Condition and trigger tools
- Georges : Edit the monsters, the primitives description and so on



WorldEditor
-----------

WorldEditor needs some directories to functionnate :
ZoneBitmaps	(contains the .TGA from the max plugin)
ZoneLigos	(contains the .LIGOZONE from the max plugin)



LogicEditor
-----------

This is a stand alone tool. It does not need any system data.



Georges
-------

Georges needs a root directory which is :
DFN (This directory regroups the .DFN definition of a type class)


RESTRICTIONS
************

Do not use prim.dfn, land.dfn nor logic.dfn in georges, else if you
instanciate those you will get xxx.logic or something like that which
could be not easy to differenciate from a worldeditor logic file.