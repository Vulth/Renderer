Flat: Definitely the easiest light mode to implement, seems to be working well

Gouraud: Pretty nice, seems to be working okay

Phong: Phong lighting looks similar to gouraud lighting in 2d, but in 3d the lighting suffered
catastrophic existence failure. I wish i had more time to figure out why.

In my test of the lighting, I found that one side of the cube would change drastically while
the others wouldn't change much at all. I believe this is because the Normals of the surface
were quite different, which might explain why one side changed much more.

I wasn't certain where to put the eye without a camera command, but I assumed it would be at 
0,0,0

I used 2 grace days on Assignment 2, 1 grace day for assignment 3, 2 grace days for assignment 4