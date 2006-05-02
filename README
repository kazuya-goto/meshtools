meshtools: Mesh tools for FrontSTR

<Description>
'meshtools' includes three programs:
1. sd342to341: subdevide 342 element into 8 341 elements, trying to
               keep aspect ratio as small as possible.
2. rf341to342: refine 341 mesh into 342 mesh by introducing middle
               nodes, NOT bothering about boundary conditions on the
	       new middle nodes.
3. meshcount: count the number of nodes and elements

Note that sd342to341 and rf341to342 require high disk IO load. When
you are to convert a large scale mesh file, you'd better use the local
file system (not NFS file system).

<Compilation>
Edit Makefile and run 'make'.

<How to run>
$ sd342to341 from_file to_file
$ rf341to342 from_file to_file
$ meshcount mesh_file

<Example>
If you have a FrontSTR mesh with 342 elements, for example A342.msh,
you can obtain a fine mesh with 8 times as many elements as the
original one by the following commands:

$ sd342to341 A342.msh A342-341.msh
$ rf341to342 A342-341.msh A342-refined.msh

You can check the number of nodes and elements by

$ meshcount A342-refined.msh

<Known bugs>
If there is more than one !ELEMENT header, the program dies with
SIGSEGV. Since the mesh conversion programs assume that the mesh
consists of only one type of elements, do not use more than one header
line for !ELEMENT.


Author: Kazuya Goto <goto@nihonbashi.race.u-tokyo.ac.jp>
Date: Mar 17, 2006
